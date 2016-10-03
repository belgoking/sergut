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
#include "sergut/xml/ParseTokenType.h"

#include <string>
#include <vector>
#include <memory>

namespace sergut {
namespace xml {

/**
 * \brief The PullParser class implements a simple XML-Pull-Parser
 *
 * The PullParser works similar to a SAX parser. However, instead of having the
 * XML elements pushed to your code, you are pulling them out using the
 * \c parseNext() method. You can parse the values of the elements using the
 * \c getCurrentXXX() methods.
 *
 * This PullParser can handle partial data, i.e. you can set a save point parse
 * until you reach the end of the XML snippet, restore the save point, append
 * further data and continue parsing. This is convenient if you want to start
 * handling a document before it has completely been loaded.
 */
class PullParser
{
public:
  enum class DecodingResult: std::size_t {
    Empty      =                           0 ,
    Error      = static_cast<std::size_t>(-1),
    Incomplete = static_cast<std::size_t>(-2)
  };

  /**
   * \brief factory function for \c PullParser
   *
   * This function analyzes the data and constructs the appropriate
   * \c PullParser according to the encoding of \c data.
   * \param data XML-data that can be encoded in either UTF-8, UTF-16LE, or
   *        UTF-16BE.
   * \return a \c PullParser.
   */
  static std::unique_ptr<PullParser> createParser(const sergut::misc::ConstStringRef& data);

  /**
   * \brief factory function for \c PullParser for moving the data ownership
   * into the parser
   *
   * This function analyzes the data and constructs the appropriate
   * \c PullParser according to the encoding of \c data. The parser then
   * assumes the ownership of the given data.
   * \param data XML-data that can be encoded in either UTF-8, UTF-16LE, or
   *        UTF-16BE.
   * \return a \c PullParser.
   */
  static std::unique_ptr<PullParser> createParser(std::vector<char>&& data);

  virtual ~PullParser();

  /// \brief Export the inner XML
  virtual std::vector<char>&& extractXmlData() = 0;
  /// \brief Parse the next XML Event
  virtual ParseTokenType parseNext() = 0;
  /// \brief Get the last XML Event
  virtual ParseTokenType getCurrentTokenType() const = 0;
  /// \brief Get the name of the current XML tag
  virtual sergut::misc::ConstStringRef getCurrentTagName() const = 0;
  /// \brief Get the name of the current XML attribute
  virtual sergut::misc::ConstStringRef getCurrentAttributeName() const = 0;
  /// \brief Get the current content value. This is either an XML attribute
  ///        content or the content of an XML text node.
  virtual sergut::misc::ConstStringRef getCurrentValue() const = 0;
  /// \brief Return whether the parser is in a valid state
  bool isOk() const { return xml::isOk(getCurrentTokenType()); }

  static bool isError(DecodingResult r) noexcept { return r < DecodingResult::Empty; }
  /// \brief Append some data to the inner XML
  virtual void appendData(const char* data, const std::size_t size) = 0;
  /// \brief Set an inner save point right before the current tag
  virtual bool setSavePoint() = 0;
  /// \brief Restore the parser to the tag where the save point was set
  virtual bool restoreToSavePoint() = 0;
};

}
}
