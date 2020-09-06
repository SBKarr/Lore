/**
Copyright (c) 2020 Roman Katuntsev <sbkarr@stappler.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
**/

#include "LoreWiki.h"

NS_SA_EXT_BEGIN(lore)

static void UnitSpineIndex_followNode(Set<int64_t> &ids, UnitSpineIndex &idx, UnitSpineIndex::Node *node, size_t &max, size_t depth = 0) {
	if (ids.find(node->id) != ids.end()) {
		return;
	}

	ids.emplace(node->id);

	node->depth = depth;
	if (depth > max) {
		max = depth;
	}

	if (!node->nodes.empty()) {
		for (auto &it : node->nodes) {
			if (auto n = idx.getNode(it)) {
				UnitSpineIndex_followNode(ids, idx, const_cast<UnitSpineIndex::Node *>(n), max, depth + 1);
			}
		}
	} else if (!node->unordered.empty()) {
		for (auto &it : node->unordered) {
			if (auto n = idx.getNode(it)) {
				UnitSpineIndex_followNode(ids, idx, const_cast<UnitSpineIndex::Node *>(n), max, depth + 1);
			}
		}
	}
}

static UnitSpineIndex::Node * UnitSpineIndex_processNode(Set<int64_t> &ids, UnitSpineIndex &idx,
		UnitSpineIndex::Node *node, UnitSpineIndex::Node *prev, size_t maxDepth) {
	if (ids.find(node->id) != ids.end()) {
		return prev;
	}

	ids.emplace(node->id);

	if (node->excluded) {
		return prev;
	}
	if (node->depth == maxDepth) {
		if (node->type != UnitSpineIndex::Type::Page) {
			if (prev->depth == maxDepth && node != prev) {
				prev->next = node->id;
				node->prev = prev->id;
			}
		}
		return node;
	} else {
		if (!node->nodes.empty()) {
			for (auto &it : node->nodes) {
				if (auto n = idx.getNode(it)) {
					prev = UnitSpineIndex_processNode(ids, idx, const_cast<UnitSpineIndex::Node *>(n), prev, maxDepth);
				}
			}
		} else if (!node->unordered.empty()) {
			for (auto &it : node->unordered) {
				if (auto n = idx.getNode(it)) {
					prev = UnitSpineIndex_processNode(ids, idx, const_cast<UnitSpineIndex::Node *>(n), prev, maxDepth);
				}
			}
		}
	}
	return prev;
}

static UnitSpineIndex::Node * UnitSpineIndex_processPages(Set<int64_t> &ids, UnitSpineIndex &idx,
		UnitSpineIndex::Node *node, UnitSpineIndex::Node *prev) {
	if (ids.find(node->id) != ids.end()) {
		return prev;
	}

	ids.emplace(node->id);

	if (node->excluded) {
		return prev;
	}
	if (node->type == UnitSpineIndex::Type::Page) {
		prev->next = node->id;
		node->prev = prev->id;
		return node;
	} else {
		if (!node->nodes.empty()) {
			for (auto &it : node->nodes) {
				if (auto n = idx.getNode(it)) {
					prev = UnitSpineIndex_processPages(ids, idx, const_cast<UnitSpineIndex::Node *>(n), prev);
				}
			}
		} else if (!node->unordered.empty()) {
			for (auto &it : node->unordered) {
				if (auto n = idx.getNode(it)) {
					prev = UnitSpineIndex_processPages(ids, idx, const_cast<UnitSpineIndex::Node *>(n), prev);
				}
			}
		}
	}
	return prev;
}

static void UnitSpineIndex_fillFromData(UnitSpineIndex &idx, UnitSpineIndex::Data &data) {
	idx.nodes.reserve(data.pageObjs.size() + data.sectObjs.size() + 1);

	for (auto &it : data.pageObjs.asArray()) {
		auto &tags = it.getString("tags");
		StringView(tags).split<StringView::Chars<','>>([&] (StringView &v) {
			v.trimChars<StringView::CharGroup<CharGroupId::WhiteSpace>>();

			if (!v.empty()) {
				auto insertIt = std::lower_bound(idx.tags.begin(), idx.tags.end(), v, [] (const Pair<String, size_t> &l, const StringView &r) {
					return l.first < r;
				});
				if (insertIt == idx.tags.end()) {
					idx.tags.emplace_back(pair(v.str(), 1));
				} else if (StringView(insertIt->first) != v) {
					idx.tags.emplace(insertIt, pair(v.str(), 1));
				} else {
					++ insertIt->second;
				}
			}
		});
	}

	auto pageIt = data.pageObjs.asArray().begin();
	auto sectIt = data.sectObjs.asArray().begin();

	auto pushNode = [&] (data::Value &data, UnitSpineIndex::Type type) {
		auto root = data.getInteger("root");
		auto sect = data.getInteger("section");
		auto unit = data.getInteger("unit");

		idx.nodes.emplace_back(UnitSpineIndex::Node{data.getInteger("__oid"), root ? root : (sect ? sect : unit), 0, 0, type});
		auto &n = idx.nodes.back();

		n.data = move(data);

		if (auto &opts = n.data.getValue("options")) {
			if (opts.getBool("excludedFromSpine")) {
				n.excluded = true;
			}
		}

		auto &ord = n.data.getValue("order");
		if (ord && ord.isArray()) {
			for (auto &it : ord.asArray()) {
				if (it.getInteger()) {
					n.nodes.emplace_back(it.getInteger());
				}
			}
		}

		auto &dict = n.data.asDict();
		auto it = dict.begin();
		while (it != dict.end()) {
			if (it->first != "title" && it->first != "name" && it->first != "priority") {
				it = dict.erase(it);
			} else {
				++ it;
			}
		}
	};

	bool unitPushed = false;
	while (pageIt != data.pageObjs.asArray().end() || sectIt != data.sectObjs.asArray().end() || !unitPushed) {
		int64_t nextPageId = pageIt != data.pageObjs.asArray().end() ? pageIt->getInteger("__oid") : maxOf<int64_t>();
		int64_t nextSectId = sectIt != data.sectObjs.asArray().end() ? sectIt->getInteger("__oid") : maxOf<int64_t>();
		int64_t unitId = unitPushed ? maxOf<int64_t>() : idx.unit;

		auto m = min(min(nextPageId, nextSectId), unitId);

		if (m == unitId) {
			pushNode(data.unit, UnitSpineIndex::Type::Unit);
			unitPushed = true;
		} else if (m == nextPageId) {
			pushNode(*pageIt, UnitSpineIndex::Type::Page);
			++ pageIt;
		} else if (m == nextSectId) {
			pushNode(*sectIt, UnitSpineIndex::Type::Section);
			++ sectIt;
		}
	}

	for (auto &it : idx.nodes) {
		if (it.parent) {
			if (auto n = const_cast<UnitSpineIndex::Node *>(idx.getNode(it.parent))) {
				if (std::find(n->nodes.begin(), n->nodes.end(), it.id) == n->nodes.end()) {
					n->unordered.emplace_back(it.id);
				}
			}
		}
	}

	auto sortFn = [&] (int64_t l, int64_t r) -> bool {
		if (auto lData = idx.getNode(l)) {
			if (auto rData = idx.getNode(r)) {
				if (lData->data && rData->data) {
					return lData->data.getInteger("priority") < rData->data.getInteger("priority");
				}
			}
		}

		return l < r;
	};

	for (auto &it : idx.nodes) {
		if (!it.unordered.empty()) {
			std::sort(it.unordered.begin(), it.unordered.end(), sortFn);
		}
	}

	auto root = const_cast<UnitSpineIndex::Node *>(idx.getNode(idx.unit));

	Set<int64_t> ids;

	size_t max = 0;
	UnitSpineIndex_followNode(ids, idx, root, max);

	for (size_t i = 0; i < max; ++ i) {
		ids.clear();
		UnitSpineIndex_processNode(ids, idx, root, root, i);
	}

	ids.clear();
	UnitSpineIndex_processPages(ids, idx, root, root);

	for (auto &it : idx.nodes) {
		it.writeEncoded();
	}
}

static void UnitSpineIndex_load(UnitSpineIndex &idx) {
	UnitSpineIndex::Data data;
	data.unit = idx.units->get(idx.storage, idx.unit, {"order", "name", "title"});

	data.sectObjs = idx.sections->select(idx.storage, storage::Query()
			.select("project", data::Value(idx.unit)).order("__oid", storage::Ordering::Ascending)
			.include("order", "options", "root", "title", "priority", "tags"));

	data.pageObjs = idx.pages->select(idx.storage, storage::Query()
			.select("project", data::Value(idx.unit)).order("__oid", storage::Ordering::Ascending)
			.include("order", "options", "section", "title", "priority", "tags"));

	UnitSpineIndex_fillFromData(idx, data);
}

static void UnitSpineIndex_get(UnitSpineIndex &idx) {
	if (auto data = idx.units->get(idx.storage, idx.unit, {"spine"})) {
		if (!data.isDictionary("spine")) {
			return;
		}

		auto &spine = data.getValue("spine");
		auto &nodes = spine.getValue("nodes");
		if (nodes.isArray()) {
			idx.nodes.reserve(nodes.asArray().size());
			for (auto &it : nodes.asArray()) {
				idx.nodes.emplace_back(UnitSpineIndex::Node::decode(move(it)));
			}
		}

		auto &tags = spine.getValue("tags");
		if (tags.isArray()) {
			idx.tags.reserve(tags.asArray().size());
			for (auto &it : tags.asArray()) {
				idx.tags.emplace_back(pair(it.getString(0), it.getInteger(1)));
			}
		}
	}
}

void UnitSpineIndex::Node::writeEncoded() {
	data.setInteger(id, "id");
	data.setInteger(parent, "root");
	data.setInteger(next, "next");
	data.setInteger(prev, "prev");
	data.setInteger(depth, "depth");
	switch (type) {
	case UnitSpineIndex::Type::Unit: data.setString("unit", "type"); break;
	case UnitSpineIndex::Type::Section: data.setString("section", "type"); break;
	case UnitSpineIndex::Type::Page: data.setString("page", "type"); break;
	}
	data.setBool(excluded, "ex");

	if (!nodes.empty()) {
		auto &content = data.emplace("ord");
		for (auto &n_it : nodes) {
			content.addInteger(n_it);
		}
	}

	if (!unordered.empty()) {
		auto &content = data.emplace("uord");
		for (auto &n_it : unordered) {
			content.addInteger(n_it);
		}
	}
}

UnitSpineIndex::Node UnitSpineIndex::Node::decode(data::Value &&data) {
	Node ret;
	ret.data = move(data);

	ret.id = ret.data.getInteger("id");
	ret.parent = ret.data.getInteger("root");
	ret.next = ret.data.getInteger("next");
	ret.prev = ret.data.getInteger("prev");
	ret.depth = ret.data.getInteger("depth");

	auto &type = ret.data.getString("type");
	if (type == "unit") {
		ret.type = UnitSpineIndex::Type::Unit;
	} else if (type == "section") {
		ret.type = UnitSpineIndex::Type::Section;
	} else if (type == "page") {
		ret.type = UnitSpineIndex::Type::Page;
	}
	ret.excluded = ret.data.getBool("ex");

	if (ret.data.isArray("ord")) {
		ret.nodes.reserve(ret.data.getArray("ord").size());
		for (auto &it : ret.data.getArray("ord")) {
			if (it.isInteger()) { ret.nodes.emplace_back(it.getInteger()); }
		}
	}

	if (ret.data.isArray("uord")) {
		ret.unordered.reserve(ret.data.getArray("uord").size());
		for (auto &it : ret.data.getArray("uord")) {
			if (it.isInteger()) { ret.unordered.emplace_back(it.getInteger()); }
		}
	}

	return ret;
}

UnitSpineIndex UnitSpineIndex::create(const Request &req, int64_t unit) {
	UnitSpineIndex ret;
	ret.unit = unit;
	ret.storage = storage::Transaction::acquire(req.storage());

	auto h = req.server().getComponent<WikiComponent>();

	ret.units = &h->getProjects();
	ret.sections = &h->getSections();
	ret.pages = &h->getPages();

	UnitSpineIndex_load(ret);

	return ret;
}

UnitSpineIndex UnitSpineIndex::create(const storage::Transaction &t, int64_t unit) {
	UnitSpineIndex ret;
	ret.unit = unit;
	ret.storage = t;

	auto h = Server(mem::server()).getComponent<WikiComponent>();

	ret.units = &h->getProjects();
	ret.sections = &h->getSections();
	ret.pages = &h->getPages();

	UnitSpineIndex_load(ret);

	return ret;
}

UnitSpineIndex UnitSpineIndex::get(const Request &req, int64_t unit) {
	UnitSpineIndex ret;
	ret.unit = unit;
	ret.storage = storage::Transaction::acquire(req.storage());

	auto h = req.server().getComponent<WikiComponent>();

	ret.units = &h->getProjects();
	ret.sections = &h->getSections();
	ret.pages = &h->getPages();

	UnitSpineIndex_get(ret);

	return ret;
}

UnitSpineIndex UnitSpineIndex::get(const storage::Transaction &t, int64_t unit) {
	UnitSpineIndex ret;
	ret.unit = unit;
	ret.storage = t;

	auto h = Server(mem::server()).getComponent<WikiComponent>();

	ret.units = &h->getProjects();
	ret.sections = &h->getSections();
	ret.pages = &h->getPages();

	UnitSpineIndex_get(ret);

	return ret;
}

const UnitSpineIndex::Node *UnitSpineIndex::getNode(int64_t id) const {
	auto it = std::lower_bound(nodes.begin(), nodes.end(), id, [] (const Node &l, const int64_t &r) {
		return l.id < r;
	});
	if (it != nodes.end()) {
		return &(*it);
	}
	return nullptr;
}

data::Value UnitSpineIndex::encode() const {
	data::Value ret;

	ret.setInteger(unit, "unit");
	auto &nodesVal = ret.emplace("nodes");
	for (auto &it : nodes) {
		nodesVal.addValue(it.data);
	}

	auto &tagsVal = ret.emplace("tags");
	for (auto &it : tags) {
		auto &v = tagsVal.emplace();
		v.addString(it.first);
		v.addInteger(it.second);
	}

	return ret;
}

NS_SA_EXT_END(lore)
