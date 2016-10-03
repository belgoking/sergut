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

#include "sergut/xml/PullParser.h"

#include "sergut/unicode/Utf16Codec.h"
#include "sergut/unicode/Utf8Codec.h"
#include "sergut/xml/detail/PullParserUtf16BE.h"
#include "sergut/xml/detail/PullParserUtf16LE.h"
#include "sergut/xml/detail/PullParserUtf8.h"

sergut::xml::PullParser::~PullParser() { }

std::unique_ptr<sergut::xml::PullParser> sergut::xml::PullParser::createParser(const sergut::misc::ConstStringRef& data)
{
  if(sergut::unicode::Utf16BECodec::hasBom(data.begin(), data.end())) {
    return std::unique_ptr<sergut::xml::detail::PullParserUtf16BE>(new sergut::xml::detail::PullParserUtf16BE(sergut::misc::ConstStringRef(data.begin()+2, data.end())));
  }
  if(sergut::unicode::Utf16LECodec::hasBom(data.begin(), data.end())) {
    return std::unique_ptr<sergut::xml::detail::PullParserUtf16LE>(new sergut::xml::detail::PullParserUtf16LE(sergut::misc::ConstStringRef(data.begin()+2, data.end())));
  }
  if(sergut::unicode::Utf8Codec::hasBom(data.begin(), data.end())) {
    return std::unique_ptr<sergut::xml::detail::PullParserUtf8>(new sergut::xml::detail::PullParserUtf8(sergut::misc::ConstStringRef(data.begin()+3, data.end())));
  }
  return std::unique_ptr<sergut::xml::detail::PullParserUtf8>(new sergut::xml::detail::PullParserUtf8(sergut::misc::ConstStringRef(data.begin(), data.end())));
}

std::unique_ptr<sergut::xml::PullParser> sergut::xml::PullParser::createParser(std::vector<char>&& data)
{
  if(sergut::unicode::Utf16BECodec::hasBom(&*data.begin(), &*data.end())) {
    return std::unique_ptr<sergut::xml::detail::PullParserUtf16BE>(new sergut::xml::detail::PullParserUtf16BE(std::move(data), 2));
  }
  if(sergut::unicode::Utf16LECodec::hasBom(&*data.begin(), &*data.end())) {
    return std::unique_ptr<sergut::xml::detail::PullParserUtf16LE>(new sergut::xml::detail::PullParserUtf16LE(std::move(data), 2));
  }
  if(sergut::unicode::Utf8Codec::hasBom(&*data.begin(), &*data.end())) {
    return std::unique_ptr<sergut::xml::detail::PullParserUtf8>(new sergut::xml::detail::PullParserUtf8(std::move(data), 3));
  }
  return std::unique_ptr<sergut::xml::detail::PullParserUtf8>(new sergut::xml::detail::PullParserUtf8(std::move(data), 0));
}
