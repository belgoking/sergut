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

#include "xml/PullParser.h"

#include "unicode/Utf16Codec.h"
#include "unicode/Utf8Codec.h"
#include "xml/detail/PullParserNVUtf16BE.h"
#include "xml/detail/PullParserNVUtf16LE.h"
#include "xml/detail/PullParserNVUtf8.h"

xml::PullParser::~PullParser() { }

std::unique_ptr<xml::PullParser> xml::PullParser::createParser(const misc::StringRef& data)
{
  if(unicode::Utf16BECodec::hasBom(data.begin(), data.end())) {
    return std::unique_ptr<xml::detail::PullParserNVUtf16BE>(new xml::detail::PullParserNVUtf16BE(misc::StringRef(data.begin()+2, data.end())));
  }
  if(unicode::Utf16LECodec::hasBom(data.begin(), data.end())) {
    return std::unique_ptr<xml::detail::PullParserNVUtf16LE>(new xml::detail::PullParserNVUtf16LE(misc::StringRef(data.begin()+2, data.end())));
  }
  if(unicode::Utf8Codec::hasBom(data.begin(), data.end())) {
    return std::unique_ptr<xml::detail::PullParserNVUtf8>(new xml::detail::PullParserNVUtf8(misc::StringRef(data.begin()+3, data.end())));
  }
  return std::unique_ptr<xml::detail::PullParserNVUtf8>(new xml::detail::PullParserNVUtf8(misc::StringRef(data.begin(), data.end())));
}

misc::StringRef xml::PullParser::getCurrentValue()
{
  const DecodingResult sz = getCurrentValue(nullptr, 0);
  if(isError(sz)) {
    return misc::StringRef();
  }
  currentValueBuffer.resize(static_cast<std::size_t>(sz));
  if(isError(getCurrentValue(&*currentValueBuffer.begin(), currentValueBuffer.size()))) {
    currentValueBuffer.clear();
  }
  return misc::StringRef(currentValueBuffer);
}
