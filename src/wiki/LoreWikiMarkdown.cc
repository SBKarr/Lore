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

#include "LoreWiki.h"

#include "MMDHtmlOutputProcessor.h"
#include "MMDEngine.h"
#include "MMDToken.h"
#include "MMDCore.h"

#include "hyphen.h"

NS_SA_EXT_BEGIN(lore)

class HtmlHyphOutputProcessor : public stappler::mmd::HtmlOutputProcessor {
public:
	using Engine = stappler::mmd::Engine;
	using Content = stappler::mmd::Content;
	using Token = stappler::mmd::Token;
	using token = stappler::mmd::token;
	using Extensions = stappler::mmd::Extensions;

	static void run(std::ostream *stream, mem::pool_t *pool, const StringView &str, const Rc<HyphenMap> &dict,
			const Extensions &ext = stappler::mmd::DefaultExtensions, const data::Value *meta = nullptr) {
		Engine e; e.init(pool, str, ext);
		e.setQuotesLanguage(stappler::mmd::QuotesLanguage::Russian);

		e.process([&] (const Content &c, const StringView &s, const Token &t) {
			HtmlHyphOutputProcessor p; p.init(stream, dict, meta);
			p.process(c, s, t);
		});
	}

	virtual ~HtmlHyphOutputProcessor() { }

	virtual bool init(std::ostream *stream, const Rc<HyphenMap> &dict, const data::Value *meta) {
		if (!HtmlOutputProcessor::init(stream)) {
			return false;
		}

		_dictMap = dict;
		_meta = meta;
		html_header_level = 4;
		safeMath = true;
		return true;
	}

protected:
	virtual void pushNode(token *t, const StringView &name, InitList &&attr, VecList && vec) override {
		if (_meta && t && (name == "h1" || name == "h2" || name == "h3" || name == "h4" || name == "h5" || name == "h6" ||
				name == "p" || name == "li" || name == "pre")) {
			auto &marks = _meta->getValue("marks");
			if (marks && marks.isDictionary()) {
				auto key = toString(_nextObjectId);
				auto val = marks.getValue(key);
				if (val.isString()) {
					VecList nvec(move(vec));
					nvec.emplace_back("data-hash", val.getString());
					nvec.emplace_back("data-index", key);
					HtmlOutputProcessor::pushNode(t, name, move(attr), move(nvec));
					++ _nextObjectId;
					return;
				}
			}
			++ _nextObjectId;
		}

		HtmlOutputProcessor::pushNode(t, name, move(attr), move(vec));
	}

	void hyphenateWord(const StringViewUtf8 & wordReader) {
		if (_dictMap) {
			_dictMap->hyphenateWord(*output, wordReader);
		}
	}

	virtual void flushBuffer() override {
		if (!_dictMap) {
			HtmlOutputProcessor::flushBuffer();
		} else {
			auto bufContents = StringViewUtf8(buffer.data(), buffer.size());

			using GroupChar = StringViewUtf8::MatchCompose<StringViewUtf8::MatchCharGroup<CharGroupId::Cyrillic>,
					StringViewUtf8::MatchCharGroup<CharGroupId::Latin>>;

			*output << bufContents.readUntil<GroupChar>();

			auto wordReader = bufContents.readChars<GroupChar>();
			while (!wordReader.empty()) {
				if (wordReader.size() > 3) {
					_dictMap->hyphenateWord(*output, wordReader);
				} else {
					*output << wordReader;
				}
				*output << bufContents.readUntil<GroupChar>();
				wordReader = bufContents.readChars<GroupChar>();
			}
		}
		buffer.clear();
	}

	size_t _nextObjectId = 1;
	const data::Value *_meta = nullptr;
	Rc<HyphenMap> _dictMap = nullptr;
};


class MetaExtractor : public stappler::mmd::HtmlOutputProcessor {
public:
	using Engine = stappler::mmd::Engine;
	using Content = stappler::mmd::Content;
	using Token = stappler::mmd::Token;
	using token = stappler::mmd::token;
	using Extensions = stappler::mmd::Extensions;

	static void run(data::Value *val, mem::pool_t *pool, const StringView &str, Extensions ext = stappler::mmd::StapplerExtensions) {
		Engine e; e.init(pool, str, ext);
		e.setQuotesLanguage(stappler::mmd::QuotesLanguage::Russian);

		e.process([&] (const Content &c, const StringView &s, const Token &t) {
			MetaExtractor p; p.init(val, pool);
			p.extractBrief(s);
			p.process(c, s, t);
		});
	}

	static void run(const Callback<void(const StringView &)> &cb, mem::pool_t *pool, const StringView &str, Extensions ext = stappler::mmd::StapplerExtensions) {
		Engine e; e.init(pool, str, ext);
		e.setQuotesLanguage(stappler::mmd::QuotesLanguage::Russian);

		e.process([&] (const Content &c, const StringView &s, const Token &t) {
			MetaExtractor p; p.init(cb, pool);
			p.process(c, s, t);
		});
	}

	static void run(const Callback<bool(const StringView &, const StringView &)> &cb,
			mem::pool_t *pool, const StringView &str, Extensions ext = stappler::mmd::StapplerExtensions) {
		Engine e; e.init(pool, str, ext);
		e.setQuotesLanguage(stappler::mmd::QuotesLanguage::Russian);

		e.process([&] (const Content &c, const StringView &s, const Token &t) {
			MetaExtractor p; p.init(cb, pool);
			p.process(c, s, t);
		});
	}

	virtual ~MetaExtractor() { }

	virtual bool init(data::Value *val, mem::pool_t *pool) {
		if (!HtmlOutputProcessor::init(&_bufferStream)) {
			return false;
		}
		_value = val;
		_pool = pool;
		safeMath = true;
		return true;
	}

	virtual bool init(const Callback<void(const StringView &)> &cb, mem::pool_t *pool) {
		if (!HtmlOutputProcessor::init(&_bufferStream)) {
			return false;
		}
		_splitCallback = &cb;
		_pool = pool;
		safeMath = true;
		return true;
	}

	virtual bool init(const Callback<bool(const StringView &, const StringView &)> &cb, mem::pool_t *pool) {
		if (!HtmlOutputProcessor::init(&_bufferStream)) {
			return false;
		}
		_headlineCallback = &cb;
		_pool = pool;
		safeMath = true;
		return true;
	}

protected:
	struct ContentRecord {
		String label;
		String href;
		Vector<ContentRecord> childs;
	};

	void extractBrief(const StringView &s) {
		StringView r(s);
		r.skipChars<StringView::CharGroup<CharGroupId::WhiteSpace>>();

		while (!r.empty()) {
			if (r.is("#") || r.is('<') || r.is('$') || r.is('|')) {
				r.skipUntil<StringView::Chars<'\n'>>();
				r.skipChars<StringView::CharGroup<CharGroupId::WhiteSpace>>();
			} else {
				auto brief = r.readUntil<StringView::Chars<'\r', '\n'>>();
				mem::pool::push(_pool);
				_value->setString(brief.str(), "brief");
				mem::pool::pop();
				break;
			}
		}
	}

	void pushNode(token *t, const StringView &name, InitList &&attr, VecList && vec) override {
		flushBuffer();

		if (!_value && _splitCallback) {
			if (t && (name == "h1" || name == "h2" || name == "h3" || name == "h4" || name == "h5" || name == "h6" ||
					name == "p" || name == "li" || name == "pre")) {

				Map<search::Language, StringStream> langs;

				Token tok(t->child);
				makeTokenTreeHash([&] (const StringView &str) {
					StringViewUtf8 r(str);

					r.trimChars<StringViewUtf8::MatchCharGroup<CharGroupId::WhiteSpace>>();
					r.split<StringViewUtf8::MatchCharGroup<CharGroupId::WhiteSpace>>([&] (const StringViewUtf8 &iword) {
						StringViewUtf8 word(iword);
						word.trimUntil<
							StringViewUtf8::MatchCharGroup<CharGroupId::Alphanumeric>,
							StringViewUtf8::MatchCharGroup<CharGroupId::Cyrillic>>();
						auto lang = search::detectLanguage(StringView(word.data(), word.size()));

						StringStream *target = nullptr;
						auto it = langs.find(lang);
						if (it == langs.end()) {
							target = &langs.emplace(lang, StringStream()).first->second;
						} else {
							target = &it->second;
						}

						if (target->empty()) { (*target) << word; } else { (*target) << " " << word; }
					});
				}, t->child);

				mem::pool::push(_pool);
				for (auto &it : langs) {
					if (!it.second.empty()) {
						(*_splitCallback)(it.second.weak());
					}
				}
				mem::pool::pop();
			}
			return;
		}

		if (!_value && _headlineCallback) {
			if (!_headlineEnabled) {
				return;
			}

			if (t && (name == "h1" || name == "h2" || name == "h3" || name == "h4" || name == "h5" || name == "h6" ||
					name == "p" || name == "li" || name == "pre")) {

				StringStream tmp;

				Token tok(t->child);
				makeTokenTreeHash([&] (const StringView &str) {
					tmp << str;
				}, t->child);

				if (!tmp.empty()) {
					mem::pool::push(_pool);
					if (!(*_headlineCallback)(tmp.weak(), toString(_nextObjectId))) {
						_headlineEnabled = false;
					}
					mem::pool::pop();
				}

				tmp.clear();
				++ _nextObjectId;
			}
			return;
		}

		if (name == "a") {
			bool isInsert = false;
			StringView href;
			for (auto &it : vec) {
				if (it.first == "href" && !it.second.is('#')) {
					href = it.second;
				}
				if (it.first == "type" && it.second == "insert") {
					isInsert = true;
				}
			}
			if (href.empty()) {
				for (auto &it : attr) {
					if (it.first == "href" && !it.second.is('#')) {
						href = it.second;
					}
					if (it.first == "type" && it.second == "insert") {
						isInsert = true;
					}
				}
			}
			if (!href.empty()) {
				mem::pool::push(_pool);
				if (isInsert) {
					_value->emplace("inserts").addString(href.str());
				} else {
					_value->emplace("links").addString(href.str());
				}
				mem::pool::pop();
			}
		}

		if (t && (name == "h1" || name == "h2" || name == "h3" || name == "h4" || name == "h5" || name == "h6" ||
				name == "p" || name == "li" || name == "pre")) {
			_buffer.clear();

			Token tok(t->child);
			makeTokenTreeHash([&] (const StringView &str) {
				StackBuffer<1_KiB> tmpBuf;

				StringViewUtf8 r(str);
				while (!r.empty()) {
					auto tmp = r.readChars<StringViewUtf8::MatchCharGroup<CharGroupId::Alphanumeric>,
							StringViewUtf8::MatchCharGroup<CharGroupId::Cyrillic>>();

					tmpBuf.clear();
					if (tmp.size() < 1_KiB) {
						tmpBuf.put((const uint8_t *)tmp.data(), tmp.size());
						string::tolower_buf((char *)tmpBuf.data(), tmpBuf.size());
						_buffer.put(tmpBuf.data(), tmpBuf.size());
					}

					r.skipUntil<StringViewUtf8::MatchCharGroup<CharGroupId::Alphanumeric>,
							StringViewUtf8::MatchCharGroup<CharGroupId::Cyrillic>>();
				}
			}, t->child);

			if (!_buffer.empty()) {
				mem::pool::push(_pool);
				auto &m = (_value->hasValue("marks")?_value->getValue("marks"):_value->emplace("marks"));
				auto h = hash::hash64((const char *)_buffer.data(), _buffer.size());
				m.setString(base64url::encode(CoderSource((const uint8_t *)&h, sizeof(h))), toString(_nextObjectId));
				mem::pool::pop();
			}

			++ _nextObjectId;
		}

		tagStack.emplace_back(name, 0);
	}

	void pushInlineNode(token *t, const StringView &name, InitList &&attr, VecList && vec) override {
		flushBuffer();
	}

	void popNode() override {
		flushBuffer();
		if (_value) {
			tagStack.pop_back();
		}
	}

	virtual void flushBuffer() override {
		buffer.clear();
	}

	virtual void processHtml(const Content &c, const StringView &str, const Token &t) override {
		source = str;
		exportTokenTree(buffer, t);
		flushBuffer();

		if (!_value) {
			return;
		}

		auto &header_stack = content->getHeaders();
		if (header_stack.empty()) {
			return;
		}

		mem::pool::push(_pool);

		uint8_t level = 0;
		stappler::mmd::token * entry = nullptr;
		auto & headers = _value->emplace("headers");

		for (auto &it : header_stack) {
			entry = it;
			level = rawLevelForHeader(it);

			buffer.clear();
			exportTokenTree(buffer, entry->child);
			auto str = buffer.weak();
			auto id = labelFromHeader(source, entry);

			headers.addValue(data::Value{
				pair("level", data::Value(level)),
				pair("label", data::Value(string::trim(str))),
				pair("href", data::Value(id)),
			});
		}

		mem::pool::pop();
	}

	size_t _nextObjectId = 1;
	ContentRecord _contentsRoot;
	StringStream _bufferStream;
	data::Value *_value = nullptr;
	mem::pool_t *_pool = nullptr;
	Buffer _buffer;
	const Callback<void(const StringView &)> *_splitCallback;

	bool _headlineEnabled = true;
	const Callback<bool(const StringView &, const StringView &)> *_headlineCallback;
};

data::Value processMarkdown(mem::pool_t *pool, const StringView & source, const data::Value &cache) {
	data::Value data;
	MetaExtractor::run(&data, pool, source);
	return data;
}

void writeHtml(std::ostream *stream, mem::pool_t *pool, const StringView & source,
		const stappler::mmd::Extensions &ext) {
	stappler::mmd::HtmlOutputProcessor::run(stream, pool, source, ext);
}

void writeHtmlHyph(std::ostream *stream, mem::pool_t *pool, const StringView & source, const Rc<HyphenMap> &dict,
		const stappler::mmd::Extensions &ext, const data::Value &meta) {
	HtmlHyphOutputProcessor::run(stream, pool, source, dict, ext, meta ? &meta : nullptr);
}

void splitTextForStemmer(mem::pool_t *pool, const StringView &content, const Callback<void(const StringView &)> &cb) {
	MetaExtractor::run(cb, pool, content);
}

void splitTextForHighlight(mem::pool_t *pool, const StringView &content, const Callback<bool(const StringView &, const StringView &)> &cb) {
	MetaExtractor::run(cb, pool, content);
}

void HyphenMap::addHyphenDict(CharGroupId id, const String &file) {
	auto data = filesystem::readTextFile(file);
	if (!data.empty()) {
		auto dict = hnj_hyphen_load_data(data.data(), data.size());
		if (dict) {
			auto it = _dicts.find(id);
			if (it == _dicts.end()) {
				_dicts.emplace(id, dict);
			} else {
				hnj_hyphen_free(it->second);
				it->second = dict;
			}
		}
	}
}
Vector<uint8_t> HyphenMap::makeWordHyphens(const char16_t *ptr, size_t len) {
	if (len < 4 || len >= 255) {
		return Vector<uint8_t>();
	}

	HyphenDict *dict = nullptr;
	for (auto &it : _dicts) {
		if (inCharGroup(it.first, ptr[0])) {
			dict = it.second;
			break;
		}
	}

	if (!dict) {
		return Vector<uint8_t>();
	}

	String word = convertWord(dict, ptr, len);
	if (!word.empty()) {
		Vector<char> buf; buf.resize(word.size() + 5);

		char ** rep = nullptr;
		int * pos = nullptr;
		int * cut = nullptr;
		hnj_hyphen_hyphenate2(dict, word.data(), int(word.size()), buf.data(), nullptr, &rep, &pos, &cut);

		Vector<uint8_t> ret;
		uint8_t i = 0;
		for (auto &it : buf) {
			if (it > 0) {
				if ((it - '0') % 2 == 1) {
					ret.push_back(i + 1);
				}
			} else {
				break;
			}
			++ i;
		}
		return ret;
	}
	return Vector<uint8_t>();
}
Vector<uint8_t> HyphenMap::makeWordHyphens(const WideStringView &r) {
	return makeWordHyphens(r.data(), r.size());
}
void HyphenMap::purgeHyphenDicts() {
	for (auto &it : _dicts) {
		hnj_hyphen_free(it.second);
	}
}

void HyphenMap::hyphenateWord(std::ostream &stream, const StringViewUtf8 & wordReader) {
	auto utf16 = string::toUtf16(wordReader);

	HyphenDict *dict = nullptr;
	for (auto &it : _dicts) {
		if (inCharGroup(it.first, *wordReader)) {
			dict = it.second;
			break;
		}
	}

	if (!dict) {
		stream << wordReader;
		return;
	}

	auto koi = string::toKoi8r(utf16);
	Vector<char> buf; buf.resize(koi.size() + 5);
	char ** rep = nullptr;
	int * pos = nullptr;
	int * cut = nullptr;
	hnj_hyphen_hyphenate2(const_cast<HyphenDict *>(dict), koi.data(), int(koi.size()), buf.data(), nullptr, &rep, &pos, &cut);
	WideStringView hView(utf16);
	int offset = 0;
	int idx=0;
	for (auto &it : buf) {
		int readLen = idx-offset+1;
		auto readIdx = WideStringView(hView.data(), readLen);
		if (it > 0) {
			if ((it - '0') % 2 == 1) {
				stream << string::toUtf8(readIdx);
				stream << string::toUtf8((char16_t) 0xAD);
				hView += readLen;
				offset += readLen;
			}
		} else {
			stream << string::toUtf8(readIdx);
			break;
		}
		++ idx;
	}
}

String HyphenMap::convertWord(HyphenDict *dict, const char16_t *ptr, size_t len) {
	if (dict->utf8) {
		return string::toUtf8(WideStringView(ptr, len));
	} else {
		if (strcmp("KOI8-R", dict->cset) == 0) {
			return string::toKoi8r(WideStringView(ptr, len));
		}

		return String();
	}
}

NS_SA_EXT_END(lore)
