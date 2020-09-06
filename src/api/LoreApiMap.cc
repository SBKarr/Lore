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

#include "LoreApi.h"
#include "LoreWiki.h"
#include "LoreUsersComponent.h"

NS_SA_EXT_BEGIN(lore)

ApiHandlerMap::ApiHandlerMap() {
	using namespace db;

	auto accessControlFn = [] (HandlerMap::Handler &h) -> bool {
		if (ExternalSession::get(h.getRequest())) {
			return true;
		}
		return false;
	};

	addHandler("CreateProject", Request::Method::Post, "/createProject", accessControlFn,
			[] (HandlerMap::Handler &h) -> data::Value {
		auto &input = h.getInputFields();
		if (auto e = ExternalSession::get(h.getRequest())) {
			return ApiCall(h.getRequest()).createProject(e->getUser(), input.getString("name"), input.getString("title"));
		}
		return data::Value();
	}, data::Value({ pair("location", data::Value("target")) })).addInputFields({
		Field::Text("name", MinLength(2), MaxLength(256), Flags::Required),
		Field::Text("title", MinLength(2), MaxLength(1_KiB), Flags::Required),
	});

	addHandler("SelectProject", Request::Method::Get, "/selectProject", accessControlFn,
			[] (HandlerMap::Handler &h) -> data::Value {
		auto id = h.getQueryFields().getInteger("id");
		if (auto e = ExternalSession::get(h.getRequest())) {
			if (ApiCall(h.getRequest()).isProjectAllowed(e->getUser(), id)) {
				e->setValue(id, "project");
				return data::Value({
					pair("selected", data::Value(id))
				});
			}
		}
		return data::Value();
	}, data::Value({ pair("location", data::Value("target")) })).addQueryFields({
		Field::Integer("id", Flags::Required),
	});

	addHandler("GetSelectedProject", Request::Method::Get, "/getSelectedProject", accessControlFn,
			[] (HandlerMap::Handler &h) -> data::Value {
		if (auto e = ExternalSession::get(h.getRequest())) {
			return ApiCall(h.getRequest()).getProject(e->getUser(), e->getInteger("projects"));
		}
		return data::Value();
	}, data::Value({ pair("location", data::Value("target")) }));
}

NS_SA_EXT_END(lore)
