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

#include "sergut/unicode/ParseResult.h"
#include "sergut/unicode/Utf8Codec.h"
#include "sergut/unicode/Utf32Char.h"
#include "sergut/xml/detail/Helper.h"

#include <cassert>

namespace sergut {
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

  TextDecodingHelper(std::vector<char>& pDecodedTextBuffer, const TextType pTextType, const char* pReadPointer, const char* pReadPointerEnd)
    : originalReadPointer(pReadPointer), readPointer(pReadPointer), readPointerEnd(pReadPointerEnd)
    , decodedTextBuffer(pDecodedTextBuffer), writePointer(decodedTextBuffer.data()), textType(pTextType)
  { }

  bool decodeText();
  bool isError() const { return currentTokenType == DecodingType::Error; }
  bool isIncomplete() const { return currentTokenType == DecodingType::IncompleteText; }
  std::size_t getReadCount() const { return readPointer - originalReadPointer; }
  const char* getReadPointer() const { return readPointer; }
  const char* getEndOfTextPointer() const { return std::max(readPointer - static_cast<std::size_t>(CharDecoder::encodeChar(currentChar)),
                                                            originalReadPointer); }
  const char* getStartOfTextPointer() const { return originalReadPointer; }
  std::size_t getWriteCount() const { return writePointer - decodedTextBuffer.data(); }

private:
  bool nextChar();
  bool nextAsciiChar();
  bool handleEntity(sergut::unicode::Utf32Char* decodedCharRef);
  bool handleHexCharRef(sergut::unicode::Utf32Char* decodeCharRef);
  bool handleDecCharRef(sergut::unicode::Utf32Char* decodeCharRef);
  bool handleEntityRef(sergut::unicode::Utf32Char* decodeCharRef);
  void checkForEndChar();
  bool writeChar(const sergut::unicode::Utf32Char chr);

private:
  const char* const originalReadPointer;
  const char* readPointer;
  const char* const readPointerEnd;
  std::size_t currentCharSize = 0;
  sergut::unicode::Utf32Char currentChar = '\0';
  DecodingType currentTokenType = DecodingType::Parsing;
  std::vector<char>& decodedTextBuffer;
  char* writePointer;
  TextType textType;
};

} // namespace detail
} // namespace xml
} // namespace sergut

template<typename CharDecoder>
inline
bool sergut::xml::detail::TextDecodingHelper<CharDecoder>::decodeText()
{
  if(readPointer == readPointerEnd) {
    // TextType::Plain is mainly for unit-tests where we want to be able
    // to decode text without having to add dummy characters at the end
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
      decodedTextBuffer.resize(writePointer - decodedTextBuffer.data());
      return true;
    case DecodingType::Parsing:
      break;
    }
    sergut::unicode::Utf32Char chr;
    if(currentChar == '&') {
      if(!handleEntity(&chr)) {
        return false;
      }
    } else {
      chr = currentChar;
    }
    if(!sergut::xml::detail::Helper::isValidXmlChar(chr)) {
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
  // chop off the part of decodedTextBuffer that is beyond writePointer
  decodedTextBuffer.resize(writePointer - decodedTextBuffer.data());
  return currentTokenType == DecodingType::AtEnd;
}

template<typename CharDecoder>
inline
bool sergut::xml::detail::TextDecodingHelper<CharDecoder>::nextChar()
{
  if(readPointer == readPointerEnd) {
    currentTokenType = DecodingType::IncompleteText;
    return false;
  }
  const sergut::unicode::ParseResult parseResult =
      CharDecoder::parseNext(currentChar, readPointer, readPointerEnd);
  if(!sergut::unicode::isError(parseResult)) {
    readPointer += std::size_t(parseResult);
    return true;
  }
  switch (parseResult) {
  case sergut::unicode::ParseResult::IncompleteCharacter:
    currentTokenType = DecodingType::IncompleteText;
    return false;
  case sergut::unicode::ParseResult::InvalidCharacter:
    currentTokenType = DecodingType::Error;
    return false;
  }
  assert(false);
}

template<typename CharDecoder>
inline
bool sergut::xml::detail::TextDecodingHelper<CharDecoder>::nextAsciiChar()
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
namespace sergut {
namespace xml {
namespace detail {
template<>
inline
bool TextDecodingHelper<sergut::unicode::Utf8Codec>::nextAsciiChar()
{
  if(readPointer == readPointerEnd) {
    currentTokenType = DecodingType::IncompleteText;
    return false;
  }
  if(!sergut::unicode::Utf8Codec::isAscii(*readPointer)) {
    currentTokenType = DecodingType::Error;
    return false;
  }
  currentChar = *readPointer;
  ++readPointer;
  return true;
}
}
}
}

template<typename CharDecoder>
inline
bool sergut::xml::detail::TextDecodingHelper<CharDecoder>::handleEntity(sergut::unicode::Utf32Char* decodedCharRef)
{
  // assume I am on the '&' currently
  if(!nextAsciiChar()) { return false; }
  sergut::unicode::Utf32Char tmpChar;
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
bool sergut::xml::detail::TextDecodingHelper<CharDecoder>::handleHexCharRef(sergut::unicode::Utf32Char* decodeCharRef)
{
  uint cnt = 0;
  sergut::unicode::Utf32Char tmpCodePoint = 0;
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
bool sergut::xml::detail::TextDecodingHelper<CharDecoder>::handleDecCharRef(sergut::unicode::Utf32Char* decodeCharRef)
{
  uint cnt = 0;
  sergut::unicode::Utf32Char tmpCodePoint = 0;
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
bool sergut::xml::detail::TextDecodingHelper<CharDecoder>::handleEntityRef(sergut::unicode::Utf32Char* decodeCharRef)
{
  // amp, apos, gt, lt, quot
  sergut::unicode::Utf32Char tmpCodePoint = sergut::unicode::Utf32Char(-1);
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
  if(tmpCodePoint == sergut::unicode::Utf32Char(-1)) {
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
void sergut::xml::detail::TextDecodingHelper<CharDecoder>::checkForEndChar()
{
  switch(textType) {
  case TextType::Plain:
    // don't do anything as everything should have been checked in the first pass
    break;
  case TextType::AttValueApos:
  case TextType::AttValueQuote:
  {
    const sergut::unicode::Utf32Char attributeChar = textType == TextType::AttValueApos ? '\'' : '"';
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
bool sergut::xml::detail::TextDecodingHelper<CharDecoder>::writeChar(const sergut::unicode::Utf32Char chr)
{
  // ensure there is enough spare space in decodedTextBuffer
  if(&*decodedTextBuffer.end() - writePointer < 4) {
    std::size_t writePointerOffset = writePointer - decodedTextBuffer.data();
    try {
      decodedTextBuffer.resize(decodedTextBuffer.size() + 50);
    } catch(const std::exception&) {
      currentTokenType = DecodingType::Error;
      return false;
    }

    writePointer = decodedTextBuffer.data() + writePointerOffset;
  }

  // then decode the current character to decodedTextBuffer
  sergut::unicode::ParseResult r = sergut::unicode::Utf8Codec::encodeChar(chr, writePointer, &*decodedTextBuffer.end());
  if(sergut::unicode::isError(r)) {
    switch(r) {
    case sergut::unicode::ParseResult::IncompleteCharacter:
      currentTokenType = DecodingType::IncompleteText;
      break;
    case sergut::unicode::ParseResult::InvalidCharacter:
      currentTokenType = DecodingType::Error;
      break;
    }
    return false;
  }
  writePointer += std::size_t(r);
  return true;
}
