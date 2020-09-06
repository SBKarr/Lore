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

#include "LoreUsersComponent.h"
#include "SPugContext.h"

NS_SA_EXT_BEGIN(lore)

class IndexRenderHandler : public RequestHandler {
public:
	virtual bool isRequestPermitted(Request & rctx) override;
	virtual int onTranslateName(Request &rctx) override;
};

bool IndexRenderHandler::isRequestPermitted(Request & rctx) {
	return true;
}

int IndexRenderHandler::onTranslateName(Request &rctx) {
	rctx.runPug("templates/auth-user.pug", [&] (pug::Context &exec, const pug::Template &) -> bool {
		if (auto longSession = LongSession::acquire(rctx)) {
			exec.set("longSessionUserId", data::Value(longSession->getUser()));
		}

		if (auto session = ExternalSession::acquire(rctx)) {
			exec.set("sessionUuid", data::Value(session->getUuid().str()));

			auto u = session->getUser();
			exec.set("sessionUserId", data::Value(u));
			auto userScheme = rctx.server().getScheme("external_users");
			if (u && userScheme) {
				auto t = storage::Transaction::acquire(rctx.storage());
				if (auto user = userScheme->get(t, u)) {
					if (auto provs = userScheme->getProperty(t, u, "providers")) {
						user.setValue(move(provs), "providers");
					}
					exec.set("user", data::Value(move(user)));
				}
			}
		}
		return true;
	});
	return DONE;
}

NS_SA_EXT_END(lore)
