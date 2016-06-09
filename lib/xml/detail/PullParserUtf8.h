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

#pragma once

#include "unicode/Utf8Codec.h"
#include "xml/detail/BasicPullParser.h"

namespace xml {
namespace detail {

class PullParserUtf8: public BasicPullParser<unicode::Utf8Codec>
{
  using BasicPullParser<unicode::Utf8Codec>::BasicPullParser;
};

template<>
inline
bool BasicPullParser<unicode::Utf8Codec>::nextAsciiChar()
{
  if(readerState.readPointer == &*inputData.end()) {
    incompleteDocument = true;
    return false;
  }
  if(!unicode::Utf8Codec::isAscii(*readerState.readPointer)) {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  readerState.currentChar = *readerState.readPointer;
  ++readerState.readPointer;
  return true;
}

template<>
bool BasicPullParser<unicode::Utf8Codec>::parseName(const NameType nameType)
{
  // [4] NameStartChar ::=  ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] |
  //                        [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] |
  //                        [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
  // [4a] NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
  if(!Helper::isNameStartChar(readerState.currentChar)) return false;
  const char* startOfName = readerState.readPointer - 1;
  if(!nextChar()) return true;
  while(Helper::isNameChar(readerState.currentChar)) {
    if(!nextChar()) return true;
  }
  switch(nameType) {
  case NameType::Tag:
    decodedNameBuffers.decodedTagName = misc::ConstStringRef(startOfName, readerState.readPointer - 1);
//    parseStack.pushData(decodedNameBuffers.decodedTagName);
    return true;
  case NameType::Attribute:
    decodedNameBuffers.decodedAttrName = misc::ConstStringRef(startOfName, readerState.readPointer - 1);
    return true;
  }
  assert(false);
  return true;
}

}
}
