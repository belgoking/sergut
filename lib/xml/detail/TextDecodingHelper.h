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

#include "unicode/ParseResult.h"
#include "unicode/Utf8Codec.h"
#include "unicode/Utf32Char.h"
#include "xml/detail/Helper.h"

#include <cassert>

namespace xml {
namespace detail {

template<typename CharDecoder>
class TextDecodingHelper {
public:
  typedef CharDecoder CharDecoderT;
  enum class DecodingType {
    Parsing,
    AtEnd,
    IncompleteText,
    Error
  };
  enum class TextType {
    Plain,
    CharData,
    AttValueQuote,
    AttValueApos
  };

  TextDecodingHelper(char* pwritePointer, const char* const pWritePointerEnd, const TextType pTextType, const char* pReadPointer, const char* pReadPointerEnd)
    : originalReadPointer(pReadPointer), readPointer(pReadPointer), readPointerEnd(pReadPointerEnd)
    , writePointer(pwritePointer), writePointerEnd(pWritePointerEnd)
    , textType(pTextType)
  { }

  bool decodeText();
  bool isError() const { return currentTokenType == DecodingType::Error; }
  bool isIncomplete() const { return currentTokenType == DecodingType::IncompleteText; }
  std::size_t getReadCount() const { return readPointer - originalReadPointer; }
  const char* getReadPointer() const { return readPointer; }
  const char* getEndOfTextPointer() const { return std::max(readPointer - static_cast<std::size_t>(CharDecoder::encodeChar(currentChar)),
                                                            originalReadPointer); }
  const char* getStartOfTextPointer() const { return originalReadPointer; }
  std::size_t getWriteCount() const { return currentWriteCount; }


private:
  bool nextChar();
  bool nextAsciiChar();
  bool handleEntity(unicode::Utf32Char* decodedCharRef);
  bool handleHexCharRef(unicode::Utf32Char* decodeCharRef);
  bool handleDecCharRef(unicode::Utf32Char* decodeCharRef);
  bool handleEntityRef(unicode::Utf32Char* decodeCharRef);
  void checkForEndChar();
  bool writeChar(const unicode::Utf32Char chr);

private:
  const char* const originalReadPointer;
  const char* readPointer;
  const char* const readPointerEnd;
  std::size_t currentCharSize = 0;
  unicode::Utf32Char currentChar = '\0';
  DecodingType currentTokenType = DecodingType::Parsing;
  char* writePointer;
  const char* const writePointerEnd;
  std::size_t currentWriteCount = 0;
  TextType textType;
};

} // namespace detail
} // namespace xml

template<typename CharDecoder>
inline
bool xml::detail::TextDecodingHelper<CharDecoder>::decodeText()
{
  if(readPointer == readPointerEnd) {
    currentTokenType = (textType == TextType::Plain) ? DecodingType::AtEnd : DecodingType::IncompleteText;
  }
  while(currentTokenType == DecodingType::Parsing) {
    if(!nextChar()) { return false; }

    checkForEndChar();
    switch(currentTokenType) {
    case DecodingType::Error:
    case DecodingType::IncompleteText:
      return false;
    case DecodingType::AtEnd:
      return true;
    case DecodingType::Parsing:
      break;
    }
    unicode::Utf32Char chr;
    if(currentChar == '&') {
      if(!handleEntity(&chr)) {
        return false;
      }
    } else {
      chr = currentChar;
    }
    if(!Helper::isValidXmlChar(chr)) {
      currentTokenType = DecodingType::Error;
      return false;
    }
    if(!writeChar(chr)) {
      return false;
    }
    if(readPointer == readPointerEnd) {
      currentTokenType = (textType == TextType::Plain) ? DecodingType::AtEnd : DecodingType::IncompleteText;
    }
  }
  return currentTokenType == DecodingType::AtEnd;
}

template<typename CharDecoder>
inline
bool xml::detail::TextDecodingHelper<CharDecoder>::nextChar()
{
  if(readPointer == readPointerEnd) {
    currentTokenType = DecodingType::IncompleteText;
    return false;
  }
  const unicode::ParseResult parseResult =
      CharDecoder::parseNext(currentChar, readPointer, readPointerEnd);
  if(!unicode::isError(parseResult)) {
    readPointer += std::size_t(parseResult);
    return true;
  }
  switch (parseResult) {
  case unicode::ParseResult::IncompleteCharacter:
    currentTokenType = DecodingType::IncompleteText;
    return false;
  case unicode::ParseResult::InvalidCharacter:
    currentTokenType = DecodingType::Error;
    return false;
  }
  assert(false);
}

template<typename CharDecoder>
inline
bool xml::detail::TextDecodingHelper<CharDecoder>::nextAsciiChar()
{
  if(!nextChar()) {
    return false;
  }
  if(currentChar > 0x7F) {
    currentTokenType = DecodingType::Error;
    return false;
  }
  return true;
}

// optimization for UTF-8
namespace xml {
namespace detail {
template<>
inline
bool TextDecodingHelper<unicode::Utf8Codec>::nextAsciiChar()
{
  if(readPointer == readPointerEnd) {
    currentTokenType = DecodingType::IncompleteText;
    return false;
  }
  if(!unicode::Utf8Codec::isAscii(*readPointer)) {
    currentTokenType = DecodingType::Error;
    return false;
  }
  currentChar = *readPointer;
  ++readPointer;
  return true;
}
}
}

template<typename CharDecoder>
inline
bool xml::detail::TextDecodingHelper<CharDecoder>::handleEntity(unicode::Utf32Char* decodedCharRef)
{
  // assume I am on the '&' currently
  if(!nextAsciiChar()) { return false; }
  unicode::Utf32Char tmpChar;
  if(currentChar == '#') {
    // parse CharRef
    // [66]   	CharRef	   ::=   	'&#' [0-9]+ ';' | '&#x' [0-9a-fA-F]+ ';'
    if(!nextAsciiChar()) { return false; }
    if(currentChar == 'x') {
      if(!nextAsciiChar()) { return false; }
      if(!handleHexCharRef(&tmpChar)) { return false; }
    } else {
      if(!handleDecCharRef(&tmpChar)) { return false; }
    }
  } else {
    // parse EntityRef
    //  [68]   	EntityRef	   ::=   	'&' Name ';'
    if(!handleEntityRef(&tmpChar)) { return false; }
  }
  if(currentChar != ';') {
    currentTokenType = DecodingType::Error;
    return false;
  }
  if(decodedCharRef) {
    *decodedCharRef = tmpChar;
  }
  return true;
}

template<typename CharDecoder>
inline
bool xml::detail::TextDecodingHelper<CharDecoder>::handleHexCharRef(unicode::Utf32Char* decodeCharRef)
{
  uint cnt = 0;
  unicode::Utf32Char tmpCodePoint = 0;
  char c = currentChar;
  while(true) {
    if(++cnt > 6) {
      // the largest unicode code point is 0x10FFFF
      currentTokenType = DecodingType::Error;
      return false;
    }
    if('0' <= c && c <= '9') {
      tmpCodePoint <<= 4;
      tmpCodePoint += c - '0';
    } else if('A' <= c && c <= 'F') {
      tmpCodePoint <<= 4;
      tmpCodePoint += c - 'A' + 10;
    } else if('a' <= c && c <= 'f') {
      tmpCodePoint <<= 4;
      tmpCodePoint += c - 'a' + 10;
    } else {
      if(cnt == 1) {
        // no numeric char
        currentTokenType = DecodingType::Error;
        return false;
      }
      if(decodeCharRef) {
        *decodeCharRef = tmpCodePoint;
        return true;
      }
    }
    if(!nextAsciiChar()) {
      return false;
    }
    c = currentChar;
  }
  currentTokenType = DecodingType::Error;
  return false;
}

template<typename CharDecoder>
inline
bool xml::detail::TextDecodingHelper<CharDecoder>::handleDecCharRef(unicode::Utf32Char* decodeCharRef)
{
  uint cnt = 0;
  unicode::Utf32Char tmpCodePoint = 0;
  char c = currentChar;
  while('0' <= c && c <= '9') {
    if(++cnt > 7) {
      // the largest unicode code point is 1114111
      currentTokenType = DecodingType::Error;
      return false;
    }
    tmpCodePoint *= 10;
    tmpCodePoint += c - '0';
    if(!nextAsciiChar()) { return false; }
    c = currentChar;
  }
  if(cnt == 0) {
    currentTokenType = DecodingType::Error;
    return false;
  }
  if(decodeCharRef) {
    *decodeCharRef = tmpCodePoint;
  }
  return true;
}

template<typename CharDecoder>
inline
bool xml::detail::TextDecodingHelper<CharDecoder>::handleEntityRef(unicode::Utf32Char* decodeCharRef)
{
  // amp, apos, gt, lt, quot
  unicode::Utf32Char tmpCodePoint = unicode::Utf32Char(-1);
  if(currentChar == 'a') {
    if(!nextAsciiChar()) { return false; }
    if(currentChar == 'm') {
      if(!nextAsciiChar()) { return false; }
      if(currentChar == 'p') {
        if(!nextAsciiChar()) { return false; }
        tmpCodePoint = '&';
      }
    } else if(currentChar == 'p') {
      if(!nextAsciiChar()) { return false; }
      if(currentChar == 'o') {
        if(!nextAsciiChar()) { return false; }
        if(currentChar == 's') {
          if(!nextAsciiChar()) { return false; }
          tmpCodePoint = '\'';
        }
      }
    }
  } else if(currentChar == 'g') {
    if(!nextAsciiChar()) { return false; }
    if(currentChar == 't') {
      if(!nextAsciiChar()) { return false; }
      tmpCodePoint = '>';
    }
  } else if(currentChar == 'l') {
    if(!nextAsciiChar()) { return false; }
    if(currentChar == 't') {
      if(!nextAsciiChar()) { return false; }
      tmpCodePoint = '<';
    }
  } else if(currentChar == 'q') {
    if(!nextAsciiChar()) { return false; }
    if(currentChar == 'u') {
      if(!nextAsciiChar()) { return false; }
      if(currentChar == 'o') {
        if(!nextAsciiChar()) { return false; }
        if(currentChar == 't') {
          if(!nextAsciiChar()) { return false; }
          tmpCodePoint = '"';
        }
      }
    }
  }
  if(tmpCodePoint == unicode::Utf32Char(-1)) {
    currentTokenType = DecodingType::Error;
    return false;
  }
  if(decodeCharRef) {
    *decodeCharRef = tmpCodePoint;
  }
  return true;
}

template<typename CharDecoder>
inline
void xml::detail::TextDecodingHelper<CharDecoder>::checkForEndChar()
{
  switch(textType) {
  case TextType::Plain:
    // don't do anything as everything should have been checked in the first pass
    break;
  case TextType::AttValueApos:
  case TextType::AttValueQuote:
  {
    const unicode::Utf32Char attributeChar = textType == TextType::AttValueApos ? '\'' : '"';
    if(currentChar == attributeChar) {
      currentTokenType = DecodingType::AtEnd;
    }
    else if(currentChar == '<') {
      currentTokenType = DecodingType::Error;
    }
    break;
  }
  case TextType::CharData:
    if(currentChar == '<') {
      currentTokenType = DecodingType::AtEnd;
    }
    break;
  }
}

template<typename CharDecoder>
inline
bool xml::detail::TextDecodingHelper<CharDecoder>::writeChar(const unicode::Utf32Char chr)
{
  unicode::ParseResult r = unicode::Utf8Codec::encodeChar(chr, writePointer, writePointerEnd);
  if(unicode::isError(r)) {
    switch(r) {
    case unicode::ParseResult::IncompleteCharacter:
      currentTokenType = DecodingType::IncompleteText;
      break;
    case unicode::ParseResult::InvalidCharacter:
      currentTokenType = DecodingType::Error;
      break;
    }
    return false;
  }
  currentWriteCount += std::size_t(r);
  if(writePointer != nullptr) {
    writePointer += std::size_t(r);
  }
  return true;
}
