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

#ifndef SRC_WIKI_LOREWIKI_H_
#define SRC_WIKI_LOREWIKI_H_

#include "Lore.h"
#include "MMDEngine.h"

// hyphenator forward declaration
typedef struct _HyphenDict HyphenDict;

NS_SA_EXT_BEGIN(lore)

enum class WikiRole {
	Nobody,
	Reader,
	Commenter,
	Writer,
	Admin
};

class WikiComponent : public ServerComponent {
public:
	WikiComponent(Server &serv, const String &name, const data::Value &dict);
	virtual ~WikiComponent() { }

	void init(const LoreComponent *);

	virtual void onChildInit(Server &) override;

	bool doContentFilter(const Scheme &, const data::Value &obj, const data::Value &origValue, data::Value &newValue);
	bool transformOrdering(const db::Scheme &scheme, data::Value &val);

	const Scheme & getProjects() const { return _projects; }
	const Scheme & getSections() const { return _sections; }
	const Scheme & getPages() const { return _pages; }
	const Scheme & getLocale() const { return _locale; }
	const Scheme & getImages() const { return _images; }
	const Scheme & getWarnings() const { return _warnings; }
	const Scheme & getGrants() const { return _grants; }

protected:
	int64_t getProjectIdForSection(int64_t) const;
	int64_t getProjectIdForPage(int64_t) const;

	Scheme _projects = Scheme("lore_projects");
	Scheme _sections = Scheme("lore_sections");
	Scheme _pages = Scheme("lore_pages");
	Scheme _locale = Scheme("lore_locale");
	Scheme _images = Scheme("lore_images");
	Scheme _warnings = Scheme("lore_warnings");
	Scheme _grants = Scheme("lore_wiki_grants");
};

struct UnitSpineIndex {
	using VectorIndex = Vector<Pair<int64_t, const data::Value *>>;

	enum class Type {
		Unit,
		Section,
		Page
	};

	struct Data {
		data::Value unit;
		data::Value sectObjs;
		data::Value pageObjs;
	};

	struct Node {
		int64_t id = 0;
		int64_t parent = 0;
		int64_t prev = 0;
		int64_t next = 0;
		Type type = Type::Section;
		bool excluded = false;

		Vector<int64_t> nodes;
		Vector<int64_t> unordered;

		data::Value data;

		size_t depth = 0;

		static Node decode(data::Value &&);
		void writeEncoded();
	};

	static UnitSpineIndex create(const Request &, int64_t unit);
	static UnitSpineIndex create(const storage::Transaction &, int64_t unit);

	static UnitSpineIndex get(const Request &, int64_t unit);
	static UnitSpineIndex get(const storage::Transaction &, int64_t unit);

	const Node *getNode(int64_t) const;

	data::Value encode() const;

	int64_t unit = 0;
	Vector<Node> nodes;
	Vector<Pair<String, size_t>> tags;

	const db::Scheme *units = nullptr;
	const db::Scheme *sections = nullptr;
	const db::Scheme *pages = nullptr;
	db::Transaction storage = nullptr;
};

class HyphenMap : public SharedObject {
public:
	void addHyphenDict(CharGroupId id, const String &file);
	Vector<uint8_t> makeWordHyphens(const char16_t *ptr, size_t len);
	Vector<uint8_t> makeWordHyphens(const WideStringView &);
	void purgeHyphenDicts();

	void hyphenateWord(std::ostream &, const StringViewUtf8 & wordReader);

	HyphenMap(mem::pool_t *p) : SharedObject(p) { }
	virtual ~HyphenMap() { }

protected:
	String convertWord(HyphenDict *, const char16_t *ptr, size_t len);

	Map<CharGroupId, HyphenDict *> _dicts;
};

data::Value processMarkdown(mem::pool_t *pool, const StringView & source, const data::Value &cache);

void writeHtml(std::ostream *, mem::pool_t *, const StringView & source,
		const stappler::mmd::Extensions &ext = stappler::mmd::DefaultExtensions);
void writeHtmlHyph(std::ostream *, mem::pool_t *, const StringView & source, const Rc<HyphenMap> &,
		const stappler::mmd::Extensions &ext = stappler::mmd::DefaultExtensions, const data::Value &meta = data::Value());

void splitTextForStemmer(mem::pool_t *pool, const StringView &content, const Callback<void(const StringView &)> &);

void splitTextForHighlight(mem::pool_t *pool, const StringView &content, const Callback<bool(const StringView &, const StringView &)> &);

NS_SA_EXT_END(lore)

#endif /* SRC_WIKI_LOREWIKI_H_ */
