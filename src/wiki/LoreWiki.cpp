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
#include "LoreWiki.h"
#include "LoreUsersComponent.h"

#include "LoreWikiMarkdown.cc"
#include "LoreWikiSpine.cc"

NS_SA_EXT_BEGIN(lore)

WikiComponent::WikiComponent(Server &serv, const String &name, const data::Value &dict)
: ServerComponent(serv, name.empty()?"Wiki":name, dict) {
	exportValues(_projects, _sections, _pages, _locale, _images, _warnings, _grants);
}

void WikiComponent::init(const LoreComponent *c) {
	using namespace db;

	auto contentFields = Vector<Field>({
		Field::Text("type"),
		Field::Text("content", MinLength(2), MaxLength(100_KiB)),
		Field::Data("meta"),
		Field::Integer("mtime"),
	});

	auto commonFields = Vector<Field>({
		Field::Boolean("hidden", Flags::ForceInclude),

		Field::Integer("ctime", Flags::ReadOnly | Flags::AutoCTime | Flags::ForceInclude),
		Field::Integer("mtime", Flags::ReadOnly | Flags::AutoMTime | Flags::Indexed | Flags::ForceInclude),

		Field::Text("name", Transform::Identifier, Flags::Required | Flags::Indexed | Flags::ForceInclude),
		Field::Bytes("uuid", Transform::Uuid, Flags::Indexed | Flags::Unique | Flags::ForceInclude),

		Field::Text("title", MinLength(2), MaxLength(1_KiB), Flags::ForceInclude),
		Field::Text("tags", MaxLength(1_KiB)),

		Field::Set("images", _images, Flags::Composed),
	});


	_projects.define(Vector<Field>(commonFields), Vector<Field>({
		Field::Text("defaultLanguage", MinLength(2), MaxLength(10), Transform::Identifier, Flags::Indexed | Flags::ForceInclude,
			DefaultFn([&] (const data::Value &obj) -> data::Value {
			if (auto req = Request(mem::request())) {
				if (auto s = ExternalSession::get(req)) {
					auto v = s->getString("language");
					if (!v.empty()) {
						return data::Value(v);
					}
				}
			}
			return data::Value("en");
		})),
		Field::Image("cover", MaxImageSize(1080, 1080, ImagePolicy::Resize), Vector<Thumbnail>({
			Thumbnail("thumb", 256, 256),
		})),

		Field::Data("order", Flags::ForceInclude, FilterFn([this] (const Scheme &scheme, data::Value &val) -> bool {
			return transformOrdering(scheme, val);
		})),

		Field::Data("spine", Flags::ReadOnly, Flags::ForceExclude, AutoFieldDef{
			Vector<AutoFieldScheme>({
				AutoFieldScheme( _projects, {"title", "name", "order"} ),
				AutoFieldScheme( _sections, AutoFieldScheme::ReqVec({"title", "name", "order", "root"}), AutoFieldScheme::ReqVec{"project", "root"} ),
				AutoFieldScheme( _pages, AutoFieldScheme::ReqVec({"title", "name", "section", "tags"}), AutoFieldScheme::ReqVec{"project", "section"} )
			}),
			DefaultFn([this] (const data::Value &data) -> data::Value {
				return UnitSpineIndex::create(Transaction::acquire(), data.getInteger("__oid")).encode();
			})
		}),

		Field::View("sections", _sections, ViewFn([this] (const Scheme &, const data::Value &obj) -> bool {
			return (obj.getBool("hidden") || obj.getInteger("root") != 0) ? false : true;
		}), Vector<String>{ "hidden", "root" }, storage::FieldView::Delta),

		Field::Set("locale", _locale, Flags::Composed),
		Field::Set("all_sections", _sections, Flags::Admin),
		Field::Set("all_pages", _pages, Flags::Admin),

		Field::Set("warnings", _warnings),
		Field::Set("grants", _grants),
	}),
		UniqueConstraintDef{"name", {"name"}}
	);


	_sections.define(Vector<Field>(commonFields), Vector<Field>({
		Field::Object("root", _sections, Linkage::Manual, ForeignLink("sections")),
		Field::Set("sections", _sections, Linkage::Manual, ForeignLink("root")),
		Field::Set("locale", _locale, Flags::Composed),

		Field::Data("order", Flags::ForceInclude, FilterFn([this] (const Scheme &scheme, data::Value &val) -> bool {
			return transformOrdering(scheme, val);
		})),

		Field::Object("project", _projects, RemovePolicy::Cascade, Flags::Required, DefaultFn([this] (const data::Value &val) -> data::Value {
			if (auto id = getProjectIdForSection(val.getInteger("root"))) { return data::Value(id); }
			return data::Value();
		})),

		Field::View("pages", _pages, ViewFn([this] (const Scheme &, const data::Value &obj) -> bool {
			return obj.getBool("hidden") ? false : true;
		}), Vector<String>({ "hidden" }), storage::FieldView::Delta),

		Field::Set("all_pages", _pages),
	}));


	_pages.define(Vector<Field>(commonFields), Vector<Field>({
		Field::Set("locale", _locale, Flags::Composed),

		Field::Object("project", _projects, RemovePolicy::Cascade, Flags::Required, DefaultFn([this] (const data::Value &val) -> data::Value {
			if (auto id = getProjectIdForSection(val.getInteger("section"))) { return data::Value(id); }
			return data::Value();
		})),
		Field::Object("section", _sections, RemovePolicy::Cascade, Flags::Required),
	}));


	_locale.define(Vector<Field>({
		Field::Integer("ctime", Flags::ReadOnly | Flags::AutoCTime | Flags::ForceInclude),
		Field::Integer("mtime", Flags::ReadOnly | Flags::AutoMTime | Flags::Indexed | Flags::ForceInclude),
		Field::Text("language", MinLength(2), MaxLength(10), Transform::Identifier, Flags::Required | Flags::Indexed | Flags::ForceInclude),

		Field::Text("title", MinLength(2), MaxLength(1_KiB), Flags::ForceInclude),
		Field::Text("tags", MaxLength(1_KiB)),
		Field::Integer("origin"),

		Field::Extra("content", Flags::ForceExclude, Vector<Field>(contentFields),
				ReplaceFilterFn([this] (const Scheme &scheme, const data::Value &obj, const data::Value &origVal, data::Value &newVal) -> bool {
			return doContentFilter(scheme, obj, origVal, newVal);
		})),

		Field::Set("images", _images, Flags::Composed),
		Field::Image("cover", MaxImageSize(1080, 1080, ImagePolicy::Resize), Vector<Thumbnail>({
			Thumbnail("cover512", 512, 512),
			Thumbnail("cover256", 256, 256),
			Thumbnail("cover128", 128, 128),
			Thumbnail("cover64", 64, 64),
		})),
	}),
		UniqueConstraintDef{"language_origin", {"language", "origin"}}
	);


	_images.define(Vector<Field>({
		Field::Integer("ctime", Flags::ReadOnly | Flags::AutoCTime | Flags::ForceInclude),
		Field::Integer("mtime", Flags::ReadOnly | Flags::AutoMTime | Flags::Indexed | Flags::ForceInclude),

		Field::Integer("origin"),
		Field::Text("name", Transform::Identifier, Flags::Required | Flags::Indexed | Flags::ForceInclude),
		Field::Text("title", MinLength(2)),
		Field::Text("tags"),

		Field::Image("content", MaxImageSize(2048, 2048, storage::ImagePolicy::Resize), Vector<Thumbnail>({
			Thumbnail("thumb", 380, 380)
		})),
	}));


	_warnings.define({
		Field::Integer("target", Flags::Indexed),
		Field::Integer("mtime", Flags::ReadOnly | Flags::AutoMTime),
		Field::Boolean("removed"),
		Field::Text("name", Transform::Identifier, Flags::Indexed),
		Field::Text("title", MinLength(2), MaxLength(1_KiB)),
		Field::Text("info", MinLength(2), MaxLength(10_KiB)),

		Field::Object("page", _pages, RemovePolicy::Cascade),
		Field::Object("project", _projects),
	});


	_grants.define(Vector<Field>({
		Field::Object("user", c->getUsers()->getExternalUserScheme(), RemovePolicy::Cascade),
		Field::Object("project", _projects),
		Field::Integer("role", data::Value(toInt(WikiRole::Nobody))),
	}),
		UniqueConstraintDef{"user_project", {"user", "project"}}
	);
}

void WikiComponent::onChildInit(Server &serv) {

}

bool WikiComponent::doContentFilter(const Scheme &scheme, const data::Value &obj, const data::Value &origValue, data::Value &newValue) {
	if (newValue.getString("type") == "text/markdown") {
		newValue.erase("meta");
		newValue.setValue(processMarkdown(Request(mem::request()).pool(), newValue.getString("content"),
			(origValue.getString("type") == "text/markdown") ? origValue.getValue("meta") : data::Value()), "meta");
		newValue.setInteger(Time::now().toMicroseconds(), "mtime");
	} else if (newValue.getString("type") == "text/markdown;forced") {
		newValue.setString("text/markdown", "type");
		return true;
	} else {
		newValue.erase("meta");
	}
	return true;
}

bool WikiComponent::transformOrdering(const db::Scheme &scheme, data::Value &val) {
	if (val.isArray()) {
		auto &arr = val.asArray();
		auto it = arr.begin();
		while (it != arr.end()) {
			if (!it->isInteger()) {
				if (auto id = it->asInteger()) {
					it->setInteger(id);
					++ it;
				} else {
					it = arr.erase(it);
				}
			} else {
				++ it;
			}
		}
		return true;
	}
	return false;
}

int64_t WikiComponent::getProjectIdForSection(int64_t id) const {
	if (id) {
		if (auto storage = storage::Adapter::FromContext()) {
			auto val = storage::Worker(_sections, storage).get(id, {"project"});
			if (auto projId = val.getInteger("project")) {
				return projId;
			}
		}
	}
	return 0;
}

int64_t WikiComponent::getProjectIdForPage(int64_t id) const {
	if (id) {
		if (auto storage = storage::Adapter::FromContext()) {
			auto val = storage::Worker(_pages, storage).get(id, {"project"});
			if (auto projId = val.getInteger("project")) {
				return projId;
			}
		}
	}
	return 0;
}

NS_SA_EXT_END(lore)
