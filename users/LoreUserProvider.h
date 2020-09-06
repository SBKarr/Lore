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

#ifndef SRC_USERS_USERPROVIDER_H_
#define SRC_USERS_USERPROVIDER_H_

#include "Lore.h"
#include "JsonWebToken.h"

NS_SA_EXT_BEGIN(lore)

class DiscoveryDocument : public SharedObject {
public:
	using KeyData = JsonWebToken::KeyData;

	virtual bool init(const data::Value &);

	const String &getIssuer() const;
	const String &getTokenEndpoint() const;
	const String &getAuthorizationEndpoint() const;

	const KeyData *getKey(const StringView &) const;

	DiscoveryDocument(mem::pool_t *p) : SharedObject(p) { }

protected:
	String issuer;
	String authorizationEndpoint;
	String tokenEndpoint;
	String jwksEndpoint;

	Map<String, KeyData> keys;
	uint64_t mtime = 0;
};

class Provider : public AllocPool {
public:
	using Scheme = storage::Scheme;
	using Transaction = storage::Transaction;

	static Provider *create(const StringView &name, const data::Value &);

	virtual ~Provider() { }

	virtual String onLogin(Request &rctx, const StringView &state, const StringView &redirectUrl) const = 0;
	virtual String onTry(Request &rctx, const data::Value &data, const StringView &state, const StringView &redirectUrl) const = 0;
	virtual uint64_t onResult(Request &rctx, const StringView &redirectUrl) const = 0;
	virtual uint64_t onToken(Request &rctx, const StringView &tokenStr) const = 0;

	virtual data::Value authUser(Request &rctx, const StringView &sub, data::Value &&userPatch) const;

	virtual void update(Mutex &);

	const data::Value &getConfig() const;

	virtual String getUserId(const data::Value &) const;

	virtual data::Value initUser(int64_t currentUser, const storage::Transaction &t, data::Value &&userPatch) const;
	virtual data::Value initUser(int64_t currentUser, const storage::Transaction &t, const StringView &, data::Value &&userPatch, int *status = nullptr) const;
	virtual data::Value makeProviderPatch(data::Value &&) const;

protected:
	data::Value makeUserPatch(const JsonWebToken &token) const;

	Provider(const StringView &, const data::Value &);

	data::Value _config;
	String _name;
	String _type;
	String _clientId;
	String _clientSecret;
};

NS_SA_EXT_END(lore)

#endif /* SRC_USERS_USERPROVIDER_H_ */
