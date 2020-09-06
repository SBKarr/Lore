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

#ifndef SRC_INDEX_LOREINDEX_H_
#define SRC_INDEX_LOREINDEX_H_

#include "Lore.h"
#include "SPugCache.h"
#include "SPugContext.h"
#include "ContinueToken.h"

NS_SA_EXT_BEGIN(lore)

class IndexHandlerInterface : public HandlerMap::Handler {
public:
	IndexHandlerInterface() { }

	virtual bool isPermitted() override;

protected:
	virtual data::Value getBreadcrumbs() const;
	virtual data::Value getUserNavigation() const;

	void defineTemplateContext(pug::Context &exec);
	void defineErrors(pug::Context &exec);

	const LoreComponent *_component = nullptr;
	db::Transaction _transaction = nullptr;
};

NS_SA_EXT_END(lore)

#endif /* SRC_INDEX_LOREINDEX_H_ */
