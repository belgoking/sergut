/* Copyright (c) 2016 Tobias Koelsch
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "sergut/UrlSerializeToVector.h"

#include "sergut/Misc.h"

#include <iomanip>
#include <map>
#include <vector>

namespace sergut {

UrlSerializeToVector::UrlSerializeToVector(std::unique_ptr<UrlNameCombiner>&& nameCombiner)
  : _ownUrlNameCombiner(nameCombiner ? std::move(nameCombiner) : std::unique_ptr<UrlNameCombiner>(new UrlNameCombiner))
  , _out(_ownOut)
  , _urlNameCombiner(*_ownUrlNameCombiner)
  , _seenNames(_ownSeenNames)
{ }

UrlSerializeToVector::UrlSerializeToVector(const UrlSerializeToVector &ref, const misc::ConstStringRef memberName)
  : _out(ref._out)
  , _urlNameCombiner(ref._urlNameCombiner)
  , _seenNames(ref._seenNames)
  , _structureName(_urlNameCombiner(misc::ConstStringRef(ref._structureName), memberName))
{ }

UrlSerializeToVector::~UrlSerializeToVector() { }

} // namespace sergut
