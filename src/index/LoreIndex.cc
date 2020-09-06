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

#include "LoreIndex.h"
#include "LoreApi.h"
#include "LoreUsersComponent.h"
#include "Languages.h"

NS_SA_EXT_BEGIN(lore)

class IndexHandler : public IndexHandlerInterface {
public:
	virtual int onRequest() override {
		return _request.runPug("templates/index.pug", [&] (pug::Context &exec, const pug::Template &tpl) -> bool {
			defineTemplateContext(exec);

			if (auto s = ExternalSession::get(_request)) {
				auto selected = s->getInteger("project");
				if (!selected) {
					if (auto projects = ApiCall(_request).getAvailableProjects(s->getUser())) {
						exec.set("projects", move(projects));
					}
				} else {
					if (auto proj = ApiCall(_request).getProject(s->getUser(), selected)) {
						exec.set("project", move(proj));
					}
				}
			}

			defineErrors(exec);
			return true;
		});
	}
};

class IndexHandlerMap : public HandlerMap {
public:
	IndexHandlerMap() {
		addHandler("Index", Request::Method::Get, "/", SA_HANDLER(IndexHandler));
	}
};

bool IndexHandlerInterface::isPermitted() {
	_transaction = db::Transaction::acquire(_request.storage());
	_component = _request.server().getComponent<LoreComponent>();
	if (_component && _transaction) {
		return true;
	}
	return false;
}

data::Value IndexHandlerInterface::getBreadcrumbs() const {
	data::Value breadcrumbs;
	breadcrumbs.addValue(data::Value({
		pair("title", data::Value("LoreMechanics")),
		pair("link", data::Value(toString("/"))),
	}));
	return breadcrumbs;
}

data::Value IndexHandlerInterface::getUserNavigation() const {
	if (auto s = ExternalSession::get(_request)) {
		data::Value nav;

		auto &projs = nav.addValue(data::Value({
			pair("link", data::Value("/")),
			pair("title", data::Value("Projects"))
		}));
		auto &links = projs.emplace("nav");

		if (auto projects = ApiCall(_request).getAvailableProjects(s->getUser())) {
			for (auto &grant : projects.asArray()) {
				auto &proj = grant.getValue("project");
				links.addValue(data::Value({
					pair("link", data::Value(toString("/api/v1/selectProject?id=", proj.getInteger("__oid"), "&target=/"))),
					pair("title", data::Value(toString(proj.getString("title"), " (", proj.getString("name"), ")"))),
				}));
			}
		}

		links.addValue(data::Value({
			pair("link", data::Value("/createProject")),
			pair("title", data::Value("Create")),
		}));

		return nav;
	}
	return data::Value();
}

void IndexHandlerInterface::defineTemplateContext(pug::Context &exec) {
	exec.set("breadcrumbs", getBreadcrumbs());

	if (auto nav = getUserNavigation()) {
		exec.set("nav", move(nav));
	}

	exec.set("prettySize", [this] (pug::VarStorage &storage, pug::Var *args, size_t argc) -> pug::Var {
		if (argc == 1 && args[0].readValue().getType() == pug::Value::Type::INTEGER) {
			auto val = args[0].readValue().asInteger();
			if (val > int64_t(1_MiB)) {
				return pug::Var(pug::Value(toString(std::setprecision(4), double(val) / 1_MiB, " MiB")));
			} else if (val > int64_t(1_KiB)) {
				return pug::Var(pug::Value(toString(std::setprecision(4), double(val) / 1_KiB, " KiB")));
			} else {
				return pug::Var(pug::Value(toString(val , " bytes")));
			}
		}
		return pug::Var();
	});

	exec.set("prettyTime", [this] (pug::VarStorage &storage, pug::Var *args, size_t argc) -> pug::Var {
		if (argc == 1 && args[0].readValue().getType() == pug::Value::Type::INTEGER) {
			auto val = Time::microseconds(args[0].readValue().asInteger());
			return pug::Var(pug::Value(val.toFormat("%d %h %Y %T")));
		}
		return pug::Var();
	});
	exec.set("toInt", [this] (pug::VarStorage &storage, pug::Var *args, size_t argc) -> pug::Var {
		if (argc == 1) {
			return pug::Var(pug::Value(args[0].readValue().asInteger()));
		}
		return pug::Var();
	});

	StringStream str;
	for (auto &it : _request.getParsedQueryArgs().asDict()) {
		if (it.first != "c") {
			if (!str.empty()) {
				str << "&";
			}
			if (it.second.isArray()) {
				for (auto &iit : it.second.asArray()) {
					str << it.first << "[]=" << iit.asString();
				}
			} else {
				str << it.first << "=" << it.second.asString();
			}
		}
	}
	exec.set("customArgs", data::Value(str.str()));

	if (auto longSession = LongSession::acquire(_request)) {
		exec.set("longSessionUserId", data::Value(longSession->getUser()));
	}

	if (auto session = ExternalSession::acquire(_request)) {
		exec.set("sessionUuid", data::Value(session->getUuid().str()));

		auto u = session->getUser();
		exec.set("sessionUserId", data::Value(u));
		if (u) {
			if (auto user = _component->getUsers()->getExternalUserScheme().get(_transaction, u)) {
				if (auto provs = _component->getUsers()->getExternalUserScheme().getProperty(_transaction, u, "providers")) {
					user.setValue(move(provs), "providers");
				}
				exec.set("user", data::Value(move(user)));

				auto language = session->getString("language");
				auto locale = session->getString("locale");

				auto loc = languages::getLocale(locale);
				auto lang = languages::getLanguage(language);

				exec.set("locale", data::Value({
					pair("locale", data::Value(locale)),
					pair("localeId", data::Value(toInt(loc))),
					pair("language", data::Value(language)),
					pair("languageId", data::Value(toInt(lang))),
					pair("name", data::Value(languages::getLanguageName(lang, loc))),
				}));
			}
		}

		if (session->getRole() == db::AccessRoleId::Admin) {
			exec.set("admin", data::Value(true));
		}

	}
}

void IndexHandlerInterface::defineErrors(pug::Context &exec) {
	auto &errs = _request.getErrorMessages();
	if (!errs.empty()) {
		data::Value errors;
		for (auto &it : errs) {
			errors.addValue(it);
		}
		if (errors) {
			exec.set("errors", move(errors));
		}
	}
}

NS_SA_EXT_END(lore)
