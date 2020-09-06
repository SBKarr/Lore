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

#ifndef SRC_LORE_H_
#define SRC_LORE_H_

#include "RequestHandler.h"
#include "ServerComponent.h"
#include "Task.h"
#include "Config.h"
#include "Networking.h"
#include "ExternalSession.h"

NS_SA_EXT_BEGIN(lore)

static constexpr db::AccessRoleId NonAuthorizedUser = db::AccessRoleId::Nobody;
static constexpr db::AccessRoleId AuthorizedUser = db::AccessRoleId::Authorized;

class UsersComponent;
class WikiComponent;

class LoreComponent : public ServerComponent {
public:
	LoreComponent(Server &serv, const String &name, const data::Value &dict);
	virtual ~LoreComponent() { }

	virtual void onChildInit(Server &) override;
	virtual void onStorageTransaction(db::Transaction &t) override;
	virtual void onHeartbeat(Server &) override;

	const UsersComponent *getUsers() const { return _users; }
	const WikiComponent *getWiki() const { return _wiki; }

protected:
	Scheme _projects = Scheme("lore_projects");

	const UsersComponent *_users = nullptr;
	const WikiComponent *_wiki = nullptr;
};

NS_SA_EXT_END(lore)

#endif /* SRC_LORE_H_ */
