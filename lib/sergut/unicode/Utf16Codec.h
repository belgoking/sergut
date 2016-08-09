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

#include "sergut/misc/StringRef.h"
#include "sergut/unicode/ParseResult.h"
#include "sergut/unicode/Utf32Char.h"

#include <string>

namespace sergut {
namespace unicode {
enum class Utf16ByteOrderType {
  LittleEndian,
  BigEndian,
};

template<Utf16ByteOrderType byteOrderType>
struct BasicUtf16Codec {
  static ParseResult parseNext(Utf32Char& chr, const char* data, const char* dataEnd) noexcept;
  static unicode::ParseResult encodeChar(const Utf32Char chr, char* bufStart = nullptr, const char* bufEnd = nullptr) noexcept;
  static ParseResult appendChar(std::string& out, const Utf32Char chr);
  static bool hasBom(const char* data, const char* dataEnd) noexcept;
  static bool isSupportedEncoding(const sergut::misc::ConstStringRef& encodingStringAscii) noexcept;
};

}
}

static inline
sergut::unicode::ParseResult computeSurrogateCount(uint8_t firstChar) noexcept
{
  if((firstChar & 0xFC) == 0xD8) {
    return sergut::unicode::ParseResult(2);
  }
  if(0xD8 <= firstChar && firstChar <= 0xDF) {
    // characters from the invalid character range or the second surrogate
    return sergut::unicode::ParseResult::InvalidCharacter;
  }
  return sergut::unicode::ParseResult(1);
}

template<sergut::unicode::Utf16ByteOrderType byteOrderType>
uint8_t getHighByteAt(const std::size_t surrogatePos, const char* data);
template<> inline
uint8_t getHighByteAt<sergut::unicode::Utf16ByteOrderType::BigEndian>(const std::size_t surrogatePos, const char* data) {
  return data[2*surrogatePos];
}
template<> inline
uint8_t getHighByteAt<sergut::unicode::Utf16ByteOrderType::LittleEndian>(const std::size_t surrogatePos, const char* data) {
  return data[2*surrogatePos+1];
}

template<sergut::unicode::Utf16ByteOrderType byteOrderType>
uint8_t getLowByteAt(const std::size_t surrogatePos, const char* data);
template<> inline
uint8_t getLowByteAt<sergut::unicode::Utf16ByteOrderType::BigEndian>(const std::size_t surrogatePos, const char* data) {
  return data[2*surrogatePos+1];
}
template<> inline
uint8_t getLowByteAt<sergut::unicode::Utf16ByteOrderType::LittleEndian>(const std::size_t surrogatePos, const char* data) {
  return data[2*surrogatePos];
}


template<sergut::unicode::Utf16ByteOrderType byteOrderType>
sergut::unicode::ParseResult
sergut::unicode::BasicUtf16Codec<byteOrderType>::parseNext(sergut::unicode::Utf32Char& chr, const char* data, const char* dataEnd) noexcept
{
  if(data + 1 >= dataEnd) {
    return ParseResult::IncompleteCharacter;
  }
  Utf32Char tmpChr = 0;
  const unicode::ParseResult surrogateCount = computeSurrogateCount(getHighByteAt<byteOrderType>(0, data));
  if(isError(surrogateCount)) {
    return surrogateCount;
  }
  if(static_cast<int32_t>(surrogateCount) == 1) {
    tmpChr = getHighByteAt<byteOrderType>(0, data);
    tmpChr <<= 8;
    tmpChr |= getLowByteAt<byteOrderType>(0, data);
  } else {
    if(data + 3 >= dataEnd) {
      return ParseResult::IncompleteCharacter;
    }
    if((getHighByteAt<byteOrderType>(1, data) & 0xFC) != 0xDC) {
      // wrong start of second surrogate
      return ParseResult::InvalidCharacter;
    }
    tmpChr = getHighByteAt<byteOrderType>(0, data) & 0x03;
    tmpChr <<= 8;
    tmpChr |= getLowByteAt<byteOrderType>(0, data);
    tmpChr <<= 2;
    tmpChr |= getHighByteAt<byteOrderType>(1, data) & 0x03;
    tmpChr <<= 8;
    tmpChr |= getLowByteAt<byteOrderType>(1, data);
    tmpChr += 0x10000;
  }

  chr = tmpChr;
  return ParseResult(static_cast<int32_t>(surrogateCount) * 2);
}

template<sergut::unicode::Utf16ByteOrderType byteOrderType>
void setHighByteAt(char* data, const std::size_t surrogatePos, char value);
template<> inline
void setHighByteAt<sergut::unicode::Utf16ByteOrderType::BigEndian>(char* data, const std::size_t surrogatePos, char value) {
  data[2*surrogatePos] = value;
}
template<> inline
void setHighByteAt<sergut::unicode::Utf16ByteOrderType::LittleEndian>(char* data, const std::size_t surrogatePos, char value) {
  data[2*surrogatePos+1] = value;
}

template<sergut::unicode::Utf16ByteOrderType byteOrderType>
void setLowByteAt(char* data, const std::size_t surrogatePos, char value);
template<> inline
void setLowByteAt<sergut::unicode::Utf16ByteOrderType::BigEndian>(char* data, const std::size_t surrogatePos, char value) {
  data[2*surrogatePos+1] = value;
}
template<> inline
void setLowByteAt<sergut::unicode::Utf16ByteOrderType::LittleEndian>(char* data, const std::size_t surrogatePos, char value) {
  data[2*surrogatePos] = value;
}

template<sergut::unicode::Utf16ByteOrderType byteOrderType>
sergut::unicode::ParseResult sergut::unicode::BasicUtf16Codec<byteOrderType>::encodeChar(const unicode::Utf32Char chr, char* bufStart, const char* bufEnd) noexcept
{
  const uint bufSize = bufEnd - bufStart;
  if(0xD800 <= chr && chr <= 0xDFFF) { return ParseResult::InvalidCharacter; }
  if(chr > 0x10FFFF) { return ParseResult::InvalidCharacter; }
  if(chr <= 0xFFFF) {
    if(bufStart != nullptr) {
      if(bufSize < 2) { return ParseResult::IncompleteCharacter; }
      setHighByteAt<byteOrderType>(bufStart, 0, (chr >> 8) & 0xFF);
      setLowByteAt< byteOrderType>(bufStart, 0,  chr       & 0xFF);
    }
    return ParseResult(2);
  }
  const Utf32Char subChr = chr - 0x10000;
  if(bufStart != nullptr) {
    if(bufSize < 4) { return ParseResult::IncompleteCharacter; }
    setHighByteAt<byteOrderType>(bufStart, 0, 0xD8 |  (subChr >> 18));
    setLowByteAt< byteOrderType>(bufStart, 0,         (subChr >> 10) & 0xFF);
    setHighByteAt<byteOrderType>(bufStart, 1, 0xDC | ((subChr >>  8) & 0x03));
    setLowByteAt< byteOrderType>(bufStart, 1,          subChr        & 0xFF);
  }
  return ParseResult(4);
}

template<sergut::unicode::Utf16ByteOrderType byteOrderType>
sergut::unicode::ParseResult sergut::unicode::BasicUtf16Codec<byteOrderType>::appendChar(std::string& out, const unicode::Utf32Char chr)
{
  char buf[4];
  const ParseResult res = encodeChar(chr, buf, buf+4);
  if(isError(res)) {
    return res;
  }
  out.append(buf, buf+static_cast<int32_t>(res));
  return res;
}

template<sergut::unicode::Utf16ByteOrderType byteOrderType>
bool sergut::unicode::BasicUtf16Codec<byteOrderType>::hasBom(const char* data, const char* dataEnd) noexcept
{
  if(data+2 > dataEnd) {
    return false;
  }
  return getHighByteAt<byteOrderType>(0, data) == 0xFE
      && getLowByteAt <byteOrderType>(0, data) == 0xFF;
}

template<sergut::unicode::Utf16ByteOrderType byteOrderType>
bool sergut::unicode::BasicUtf16Codec<byteOrderType>::isSupportedEncoding(const sergut::misc::ConstStringRef& encodingStringAscii) noexcept
{
  return encodingStringAscii == sergut::misc::ConstStringRef("UTF-16");
}

namespace sergut {
namespace unicode {
struct Utf16LECodec: public sergut::unicode::BasicUtf16Codec<sergut::unicode::Utf16ByteOrderType::LittleEndian> { };
struct Utf16BECodec: public sergut::unicode::BasicUtf16Codec<sergut::unicode::Utf16ByteOrderType::BigEndian> { };
}
}
