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

#ifndef SRC_API_LOREAPI_H_
#define SRC_API_LOREAPI_H_

#include "Lore.h"

NS_SA_EXT_BEGIN(lore)

class ApiCall {
public:
	static constexpr StringView INVALID_USER = "INVALID_USER";
	static constexpr StringView GRANT_FAILED = "GRANT_FAILED";
	static constexpr StringView CREATE_FAILED = "CREATE_FAILED";
	static constexpr StringView NON_UNIQUE = "NON_UNIQUE";
	static constexpr StringView PROJECT_NOT_FOUND = "PROJECT_NOT_FOUND";

	ApiCall(const Request &);
	ApiCall(Server, const db::Transaction &);

	~ApiCall();

	data::Value createProject(int64_t user, StringView name, StringView title) const;
	data::Value getAvailableProjects(int64_t user);
	data::Value getProject(int64_t user, int64_t proj);

	data::Value getUser(int64_t) const;
	bool isProjectAllowed(int64_t user, int64_t project) const;

	const Vector<Pair<StringView, StringView>> & getError() const { return _errors; }

protected:
	void onError(StringView, StringView) const;

	Request _request;
	Server _server;
	db::Transaction _transaction = nullptr;
	UsersComponent *_users = nullptr;
	WikiComponent *_wiki = nullptr;
	LoreComponent *_lore = nullptr;
	mutable Vector<Pair<StringView, StringView>> _errors;
};

class ApiHandlerMap : public HandlerMap {
public:
	ApiHandlerMap();
};

NS_SA_EXT_END(lore)

#endif /* SRC_API_LOREAPI_H_ */
