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

#include "Define.h"
#include "LoreApi.h"
#include "LoreWiki.h"
#include "LoreUsersComponent.h"

#include "LoreApiMap.cc"

NS_SA_EXT_BEGIN(lore)

ApiCall::ApiCall(const Request &req)
: _request(req)
, _server(req.server())
, _transaction(db::Transaction::acquire(req.storage()))
, _users(_server.getComponent<UsersComponent>())
, _wiki(_server.getComponent<WikiComponent>())
, _lore(_server.getComponent<LoreComponent>()) {

}

ApiCall::ApiCall(Server serv, const db::Transaction &t)
: _server(serv)
, _transaction(t)
, _users(_server.getComponent<UsersComponent>())
, _wiki(_server.getComponent<WikiComponent>())
, _lore(_server.getComponent<LoreComponent>()) {

}

ApiCall::~ApiCall() {
	if (!_errors.empty()) {
		if (_request) {
			for (auto &it : _errors) {
				_request.addErrorMessage(data::Value({
					pair("source", data::Value("ApiCall")),
					pair("func", data::Value(it.first)),
					pair("error", data::Value(it.second))
				}));
			}
		} else {
			for (auto &it : _errors) {
				std::cout << "[Error: ApiCall] " << it.first << ": " << it.second << "\n";
			}
		}
	}
}

data::Value ApiCall::createProject(int64_t user, StringView name, StringView title) const {
	if (auto u = getUser(user)) {
		auto r = _wiki->getProjects().select(_transaction, db::Query().select("name", data::Value(name)));
		if (!r.empty()) {
			onError(__func__, NON_UNIQUE);
			return data::Value();
		}

		auto ret = _wiki->getProjects().create(_transaction, data::Value({
			pair("name", data::Value(name)),
			pair("title", data::Value(title)),
		}));

		if (ret) {
			if (auto g = _wiki->getGrants().create(_transaction, data::Value({
				pair("user", data::Value(user)),
				pair("project", data::Value(ret.getInteger("__oid"))),
				pair("role", data::Value(toInt(WikiRole::Admin)))
			}))) {
				ret.setValue(data::Value({
					data::Value(move(g))
				}), "grants");
				return ret;
			} else {
				_wiki->getProjects().remove(_transaction, ret.getInteger("__oid"));
				onError(__func__, GRANT_FAILED);
			}
		} else {
			onError(__func__, CREATE_FAILED);
		}
	}
	return data::Value();
}

data::Value ApiCall::getAvailableProjects(int64_t user) {
	if (auto u = getUser(user)) {
		auto g = _wiki->getGrants().select(_transaction, db::Query().select("user", data::Value(user)));
		if (g) {
			for (auto &it : g.asArray()) {
				if (auto proj = _wiki->getProjects().get(_transaction, it.getInteger("project"))) {
					it.setValue(proj, "project");
				} else {
					onError(__func__, PROJECT_NOT_FOUND);
				}
			}
			return g;
		}
	}
	return data::Value();
}

data::Value ApiCall::getProject(int64_t user, int64_t proj) {
	if (auto u = getUser(user)) {
		auto g = _wiki->getGrants().select(_transaction, db::Query().select("user", data::Value(user)).select("project", data::Value(proj)));
		if (g.size() == 1) {
			auto role = g.getValue(0).getInteger("role");
			if (role <= toInt(WikiRole::Nobody)) {
				onError(__func__, GRANT_FAILED);
				return data::Value();
			}

			if (auto proj = _wiki->getProjects().get(_transaction, g.getValue(0).getInteger("project"))) {
				return proj;
			} else {
				onError(__func__, PROJECT_NOT_FOUND);
			}
		} else {
			onError(__func__, GRANT_FAILED);
		}
	}
	return data::Value();
}

data::Value ApiCall::getUser(int64_t user) const {
	if (auto ret = _users->getExternalUserScheme().get(_transaction, user)) {
		return ret;
	}
	onError(__func__, INVALID_USER);
	return data::Value();
}

bool ApiCall::isProjectAllowed(int64_t user, int64_t project) const {
	auto ret = _wiki->getGrants().select(_transaction, db::Query().select("user", data::Value(user)).select("project", data::Value(project)));
	if (ret.size() == 1) {
		auto role = ret.getValue(0).getInteger("role");
		if (role > toInt(WikiRole::Nobody)) {
			return true;
		}
	}

	onError(__func__, GRANT_FAILED);
	return false;
}

void ApiCall::onError(StringView fn, StringView err) const {
	_errors.emplace_back(fn, err);
}

NS_SA_EXT_END(lore)
