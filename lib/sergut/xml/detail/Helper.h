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

#include "sergut/unicode/Utf32Char.h"

namespace sergut {
namespace xml {
namespace detail {
namespace Helper {
inline
bool isNameStartChar(sergut::unicode::Utf32Char chr) {
  if('a' <= chr && chr <= 'z') return true;
  if('A' <= chr && chr <= 'Z') return true;
  if(chr == ':' || chr == '_') return true;
  if(0xC0 <= chr && chr <= 0xD6) return true;
  if(0xD8 <= chr && chr <= 0xF6) return true;
  if(0xF8 <= chr && chr <= 0x2FF) return true;
  if(0x370 <= chr && chr <= 0x37D) return true;
  if(0x37F <= chr && chr <= 0x1FFF) return true;
  if(0x200C <= chr && chr <= 0x200D) return true;
  if(0x2070 <= chr && chr <= 0x218F) return true;
  if(0x2C00 <= chr && chr <= 0x2FEF) return true;
  if(0x3001 <= chr && chr <= 0xD7FF) return true;
  if(0xF900 <= chr && chr <= 0xFDCF) return true;
  if(0xFDF0 <= chr && chr <= 0xFFFD) return true;
  if(0x10000 <= chr && chr <= 0xEFFFF) return true;
  // [4] NameStartChar ::=  ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] |
  //                        [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] |
  //                        [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
  return false;
}

inline
bool isNameChar(sergut::unicode::Utf32Char chr) {
  if(isNameStartChar(chr)) return true;
  if('0' <= chr && chr <= '9') return true;
  if(chr == '-' || chr == '.') return true;
  if(chr == 0xB7) return true;
  if(0x300 <= chr && chr <= 0x36F) return true;
  if(0x203F <= chr && chr <= 0x2040) return true;
  // [4a] NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
  return false;
}

inline
bool isValidXmlChar(sergut::unicode::Utf32Char c)
{
  // Characters and decoded entity references must be in range
  //  [2]   	Char	   ::=   	#x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]

  if(c == 0x09 || c == 0x0A || c == 0x0D) {
    return true;
  }
  // no need to check for 0xD800-0xDFFF as the codec will reject those already
  if(0x20 <= c && c <= 0xFFFD) {
    return true;
  }
  if(0x10000 <= c && c <= 0x10FFFF) {
    return true;
  }
  return false;
}

}
}
}
}
