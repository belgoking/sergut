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

#include "sergut/UrlDeserializer.h"

#include "sergut/ParsingException.h"

namespace sergut {

UrlDeserializer::UrlDeserializer(const std::vector<std::pair<std::string, std::string> >& params,
                                 std::unique_ptr<UrlNameCombiner>&& urlNameCombiner)
  : _ownUrlNameCombiner(urlNameCombiner ? std::move(urlNameCombiner) : std::unique_ptr<UrlNameCombiner>(new UrlNameCombiner))
  , _ownParams(params)
  , _urlNameCombiner(*_ownUrlNameCombiner)
  , _params(_ownParams)
{

}

UrlDeserializer::UrlDeserializer(std::vector<std::pair<std::string, std::string> >&& params,
                                 std::unique_ptr<UrlNameCombiner>&& urlNameCombiner)
  : _ownUrlNameCombiner(urlNameCombiner ? std::move(urlNameCombiner) : std::unique_ptr<UrlNameCombiner>(new UrlNameCombiner))
  , _ownParams(std::move(params))
  , _urlNameCombiner(*_ownUrlNameCombiner)
  , _params(_ownParams)
{

}

UrlDeserializer::UrlDeserializer(const UrlDeserializer& ref, const misc::ConstStringRef memberName)
  : _urlNameCombiner(ref._urlNameCombiner)
  , _params(ref._params)
  , _structureName(_urlNameCombiner(misc::ConstStringRef(ref._structureName), memberName))
{ }


} // namespace sergut
