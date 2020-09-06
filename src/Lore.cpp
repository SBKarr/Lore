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
#include "Lore.h"
#include "LoreWiki.h"
#include "LoreUsersComponent.h"
#include "LoreApi.h"

#include "LoreIndex.cc"

NS_SA_EXT_BEGIN(lore)

LoreComponent::LoreComponent(Server &serv, const String &name, const data::Value &dict)
: ServerComponent(serv, name.empty()?"Lore":name, dict) {

	auto users = serv.addComponent(new UsersComponent(serv, "Users", dict));
	auto wiki = serv.addComponent(new WikiComponent(serv, "Wiki", dict));

	_users = users;
	_wiki = wiki;

	users->init(this);
	wiki->init(this);
}

void LoreComponent::onChildInit(Server &serv) {
	ServerComponent::onChildInit(serv);

	serv.addHandler("/", new IndexHandlerMap);
	serv.addHandler("/api/v1/", new ApiHandlerMap);
}

void LoreComponent::onStorageTransaction(db::Transaction &t) {
	if (auto req = Request(mem::request())) {
		if (auto s = ExternalSession::get(req)) {
			if (toInt(t.getRole()) < toInt(s->getRole())) {
				t.setRole(s->getRole());
			}
		}
	}
}

void LoreComponent::onHeartbeat(Server &serv) {

}

SP_EXTERN_C ServerComponent * CreateLore(Server &serv, const String &name, const data::Value &dict) {
	return new LoreComponent(serv, name, dict);
}

NS_SA_EXT_END(lore)
