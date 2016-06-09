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

#include "Utf8Codec.h"

static const misc::ConstStringRef utf8EncodingName("UTF-8");

static unicode::ParseResult computeCharSize(unicode::Utf32Char& firstCharValue, char firstChar) noexcept
{
  if((firstChar & 0x80) == 0) {
    firstCharValue = firstChar;
    return unicode::ParseResult(1);
  }
  if((firstChar & 0x40) != 0x40) {
    firstCharValue = 0;
    return unicode::ParseResult::InvalidCharacter;
  }
  if((firstChar & 0x20) == 0x00) {
    firstCharValue = firstChar & 0x1F;
    return unicode::ParseResult(2);
  }
  if((firstChar & 0x10) == 0x00) {
    firstCharValue = firstChar & 0x0F;
    return unicode::ParseResult(3);
  }
  if((firstChar & 0x08) == 0x00) {
    firstCharValue = firstChar & 0x07;
    return unicode::ParseResult(4);
  }
  firstCharValue = 0;
  return unicode::ParseResult::InvalidCharacter;
}

unicode::ParseResult unicode::Utf8Codec::parseNext(Utf32Char& chr, const char* data, const char* dataEnd) noexcept
{
  if(data == dataEnd) {
    return ParseResult::IncompleteCharacter;
  }
  Utf32Char tmpChr;
  const int32_t charSize = static_cast<int32_t>(computeCharSize(tmpChr, *data));
  if(charSize < 0) {
    return static_cast<ParseResult>(charSize);
  }
  if(static_cast<std::size_t>(charSize) > 4) {
    return ParseResult::InvalidCharacter;
  }
  if(data + static_cast<std::size_t>(charSize) > dataEnd) {
    return ParseResult::IncompleteCharacter;
  }
  for(int32_t pos = 1; pos < charSize; ++pos) {
    ++data;
    if((*data & 0xC0) != 0x80) {
      return ParseResult::InvalidCharacter;
    }
    tmpChr <<= 6;
    tmpChr |= *data & 0x3F;
  }
  switch(charSize) {
  case 2:
    if(tmpChr < 0x80) {
      return ParseResult::InvalidCharacter;
    }
    break;
  case 3:
    if(tmpChr < 0x800) {
      return ParseResult::InvalidCharacter;
    }
    break;
  case 4:
    if(tmpChr < 0x10000) {
      return ParseResult::InvalidCharacter;
    }
    break;
  }
  if(0xD800 <= tmpChr && tmpChr <= 0xDFFF) {
    return ParseResult::InvalidCharacter;
  }
  if(tmpChr > 0x10FFFF) {
    return ParseResult::InvalidCharacter;
  }

  chr = tmpChr;
  return static_cast<ParseResult>(charSize);
}

bool unicode::Utf8Codec::isAscii(const char c) noexcept
{
  return (c & 0x80) == 0;
}

unicode::ParseResult unicode::Utf8Codec::encodeChar(const unicode::Utf32Char chr, char* bufStart, const char* bufEnd) noexcept
{
  if(chr > 0x10FFFF)                 { return ParseResult::InvalidCharacter; }
  if(0xD800 <= chr && chr <= 0xDFFF) { return ParseResult::InvalidCharacter; }

  const uint bufSize = bufEnd - bufStart;
  // 0xxx xxxx
  if(chr <= 0x7F) {
    if(bufStart != nullptr) {
      if(bufSize < 1) { return ParseResult::IncompleteCharacter; }
      bufStart[0] = chr;
    }
    return ParseResult(1);
  }
  // 110x xxxx 10xx xxxx
  if(chr <= 0x07FF) {
    if(bufStart != nullptr) {
      if(bufSize < 2) { return ParseResult::IncompleteCharacter; }
      bufStart[0] = 0xC0 |  chr >> 6;
      bufStart[1] = 0x80 | (chr & 0x3F);
    }
    return ParseResult(2);
  }
  // 1110 xxxx 10xx xxxx 10xx xxxx
  if(chr <= 0xFFFF) {
    if(bufStart != nullptr) {
      if(bufSize < 3) { return ParseResult::IncompleteCharacter; }
      bufStart[0] = 0xE0 |   chr >> 12;
      bufStart[1] = 0x80 | ((chr >>  6) & 0x3F);
      bufStart[2] = 0x80 |  (chr        & 0x3F);
    }
    return ParseResult(3);
  }
  // 1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx
  if(bufStart != nullptr) {
    if(bufSize < 4) { return ParseResult::IncompleteCharacter; }
    bufStart[0] = 0xF0 |   chr >> 18;
    bufStart[1] = 0x80 | ((chr >> 12) & 0x3F);
    bufStart[2] = 0x80 | ((chr >>  6) & 0x3F);
    bufStart[3] = 0x80 |  (chr        & 0x3F);
  }
  return ParseResult(4);
}

unicode::ParseResult unicode::Utf8Codec::appendChar(std::string& out, const unicode::Utf32Char chr)
{
  char buf[4];
  const ParseResult res = encodeChar(chr, buf, buf+4);
  if(isError(res)) {
    return res;
  }
  out.append(buf, buf+static_cast<int32_t>(res));
  return res;
}


bool unicode::Utf8Codec::hasBom(const char* data, const char* dataEnd) noexcept
{
  if(data + 3 > dataEnd) {
    return false;
  }
  return data[0] == char(0xEF)
      && data[1] == char(0xBB)
      && data[2] == char(0xBF);
}

bool unicode::Utf8Codec::isSupportedEncoding(const misc::ConstStringRef& encodingString) noexcept
{
  return encodingString == utf8EncodingName;
}
