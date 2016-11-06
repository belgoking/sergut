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

#include "sergut/UrlSerializer.h"

#include "sergut/Misc.h"

#include <iomanip>
#include <map>
#include <vector>

namespace sergut {

UrlSerializer::UrlSerializer(std::unique_ptr<UrlNameCombiner>&& nameCombiner)
  : _ownOut(new std::ostringstream)
  , _ownUrlNameCombiner(nameCombiner ? std::move(nameCombiner) : std::unique_ptr<UrlNameCombiner>(new UrlNameCombiner))
  , _out(*_ownOut)
  , _urlNameCombiner(*_ownUrlNameCombiner)
  , _seenNames(_ownSeenNames)
{ }

UrlSerializer::UrlSerializer(const UrlSerializer &ref, const misc::ConstStringRef memberName)
  : _out(ref._out)
  , _urlNameCombiner(ref._urlNameCombiner)
  , _seenNames(ref._seenNames)
  , _structureName(_urlNameCombiner(misc::ConstStringRef(ref._structureName), memberName))
{ }

UrlSerializer::~UrlSerializer() { }

std::string UrlSerializer::str() const
{
  return _out.str();
}

static bool isSpecialCharacter(const char c)
{
  return !(('a' <= c && c <= 'z' )
           || ('A' <= c && c <= 'Z')
           || ('0' <= c && c <= '9')
           || c == '.'
           || c == '-'
           || c == '_');
}

static void percentEncode(char buf[4], const char c)
{
  static const char Chars[] = "0123456789abcdef";
  uint8_t tmp = static_cast<uint8_t>(c);
  buf[0] = '%';
  buf[1] = Chars[tmp>>4];
  buf[2] = Chars[tmp & 0x0f];
  buf[3] = '\0';
}

void UrlSerializer::writeEscaped(const std::string &str)
{
  char buf[] = "%ff";
  std::string::const_iterator regionStartIt = str.begin();
  std::string::const_iterator regionEndIt = str.begin();
  while(regionEndIt != str.end()) {
    if(isSpecialCharacter(*regionEndIt)) {
      // found XML-Entity character
      if(regionStartIt != regionEndIt) {
        _out.write(&*regionStartIt, regionEndIt-regionStartIt);
      }
      if(*regionEndIt == ' ') {
        _out << "+";
      } else {
        percentEncode(buf, *regionEndIt);
        _out.write(buf, 3);
      }
      ++regionEndIt;
      regionStartIt = regionEndIt;
    } else {
      ++regionEndIt;
    }
  }
  if(regionEndIt != regionStartIt) {
    _out.write(&*regionStartIt, regionEndIt-regionStartIt);
  }
}

} // namespace sergut
