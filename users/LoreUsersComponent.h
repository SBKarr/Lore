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

#ifndef SRC_USERS_USERSCOMPONENT_H_
#define SRC_USERS_USERSCOMPONENT_H_

#include "Lore.h"
#include "LoreUserProvider.h"

NS_SA_EXT_BEGIN(lore)

class UsersComponent : public ServerComponent {
public:
	static String makeStateToken();

	UsersComponent(Server &serv, const String &name, const data::Value &dict);
	virtual ~UsersComponent() { }

	void init(const LoreComponent *);

	virtual void onChildInit(Server &) override;
	virtual void onStorageTransaction(storage::Transaction &) override;

	String onLogin(Request &rctx, const StringView &provider, const StringView &state, const StringView &redirectUrl) const;
	String onTry(Request &rctx, const StringView &provider, const data::Value &data, const StringView &state, const StringView &redirectUrl) const;
	uint64_t onToken(Request &rctx, const StringView &provider, const StringView &token);
	uint64_t onResult(Request &rctx, const StringView &provider, const StringView &redirectUrl) const;

	void setNewUserCallback(Function<void(const data::Value &)> &&);

	void sendActivationEmail(Request &rctx, const data::Value &newUser) const;

	bool isLongTermAuthorized(Request &rctx, uint64_t uid) const;
	bool isSessionAuthorized(Request &rctx, uint64_t uid) const;

	bool initLongTerm(Request &rctx, uint64_t uid) const;

	bool pushAuthUser(Request &rctx, uint64_t uid) const;

	Provider *getProvider(const StringView &) const;
	Provider *getLocalProvider() const;

	StringView getPrivateKey() const;
	StringView getPublicKey() const;
	SessionKeys getKeyPair() const;

	StringView getCaptchaSecret() const;

	StringView getBindAuth() const;
	StringView getBindLocal() const;

	int64_t isValidAppRequest(Request &) const; // returns application id
	data::Value isValidSecret(Request &, const data::Value &) const;

	const Scheme & getApplicationScheme() const;
	const Scheme & getExternalUserScheme() const;
	const Scheme & getProvidersScheme() const;
	const Scheme & getLocalUserScheme() const;
	const Scheme & getConnectionScheme() const;

	data::Value getExternalUserByEmail(const StringView &) const;
	data::Value getLocaUserByName(const StringView &) const;

	bool verifyCaptcha(const StringView &captchaToken) const;

	data::Value makeConnection(Request &rctx, int64_t userId, int64_t appId, const StringView &token);

	void onNewUser(const data::Value &);

protected:
	void updateDiscovery();

	data::Value readConfig() const;

	data::Value createUserCmd(const StringView &);
	data::Value addApplicationCmd(const StringView &);

	data::Value attachUser(int64_t local, int64_t extarnal);

	using Field = storage::Field;
	using Flags = storage::Flags;
	using RemovePolicy = storage::RemovePolicy;
	using Transform = storage::Transform;
	using Scheme = storage::Scheme;
	using MaxFileSize = storage::MaxFileSize;
	using MaxImageSize = storage::MaxImageSize;
	using Thumbnail = storage::Thumbnail;

	Scheme _externalUsers = Scheme("external_users");
	Scheme _authProviders = Scheme("auth_providers");
	Scheme _localUsers = Scheme("local_users");
	Scheme _connections = Scheme("user_connections");
	Scheme _applications = Scheme("user_applications");

	String _privateKey;
	String _publicKey;
	String _captchaSecret;

	mutable Mutex _mutex;

	data::Value _config;
	Map<String, Provider *> _providers;
	Provider *_localProvider = nullptr;

	Function<void(const data::Value &)> _newUserCallback;
};

NS_SA_EXT_END(lore)

#endif /* SRC_USERS_USERSCOMPONENT_H_ */
