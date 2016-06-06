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

#include "misc/StringRef.h"
#include "xml/ParseTokenType.h"

#include <string>
#include <vector>
#include <memory>

namespace xml {

/**
 * @brief The PullParserNVUtf8 class implements a very simple XML-Pull-Parser
 *
 * It does no validating. It does not even check that closing tags match their
 * opening tags. In case you want to verify this, the using application has
 * to do so.
 */
class PullParser
{
public:
  enum class DecodingResult: std::size_t {
    Empty      =                           0 ,
    Error      = static_cast<std::size_t>(-1),
    Incomplete = static_cast<std::size_t>(-2)
  };

  static std::unique_ptr<PullParser> createParser(const misc::StringRef& data);

  virtual ~PullParser();
  virtual std::vector<char>&& extractXmlData() = 0;
  virtual ParseTokenType parseNext() = 0;
  virtual ParseTokenType getCurrentTokenType() const = 0;
  virtual misc::StringRef getCurrentTagName() const = 0;
  virtual misc::StringRef getCurrentAttributeName() const = 0;
  virtual DecodingResult getCurrentValue(char* buf, const std::size_t bufSize) const = 0;
  misc::StringRef getCurrentValue();
  bool isOk() const { return xml::isOk(getCurrentTokenType()); }

  static bool isError(DecodingResult r) noexcept { return r < DecodingResult::Empty; }

private:
  std::string currentValueBuffer;
};

}
