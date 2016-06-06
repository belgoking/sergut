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

#include <catch.hpp>

#include "unicode/Utf8Codec.h"

#include <sstream>
#include <iomanip>

TEST_CASE("Code/Decode UTF-8", "[unicode]")
{
  const std::vector<std::pair<unicode::Utf32Char, std::string>>
      chars{std::make_pair(0x00,     std::string(1, '\0')), std::make_pair(0x7F,     "\x7f"),
            std::make_pair(0x80,    "\xC2\x80"),            std::make_pair(0x7FF,    "\xDF\xBF"),
            std::make_pair(0x800,   "\xE0\xA0\x80"),        std::make_pair(0xFFFF,   "\xEF\xBF\xBF"),
            std::make_pair(0x10000, "\xF0\x90\x80\x80"),    std::make_pair(0x10FFFF, "\xF4\x8F\xBF\xBF")};
  for(const std::pair<unicode::Utf32Char, std::string> chr: chars) {
    {
      std::ostringstream txt;
      txt << "The unicode Character 0x" << std::hex << chr.first;
      GIVEN(txt.str()) {
        WHEN("The character is encoded to UTF-8") {
          std::string out;
          unicode::Utf8Codec::appendChar(out, chr.first);
          THEN("The result is the specified sequence") {
            CHECK(out == chr.second);
          }
        }
      }
    }
    {
      std::ostringstream txt;
      txt << "The UTF-8 Sequence";
      for(char c: chr.second) { txt << " 0x" << std::hex << (static_cast<uint16_t>(c)&0xFF); }
      GIVEN(txt.str()) {
        WHEN("The unicode codepoint is decoded") {
          unicode::Utf32Char c;
          const unicode::ParseResult res = unicode::Utf8Codec::parseNext(c, chr.second.data(), chr.second.data()+chr.second.size());
          THEN("The result is the specified value") {
            CHECK(res == unicode::ParseResult(chr.second.size()));
            CHECK(c == chr.first);
          }
        }
      }
    }
  }
}

TEST_CASE("Code UTF-8, check encoding errors", "[unicode]")
{
  const std::vector<unicode::Utf32Char> invalidCodepoints{
    0x110000, // too large
    0xDC00, // invalid range start
    0xDFFF, // invalid range end
  };
  for(unicode::Utf32Char c: invalidCodepoints) {
    std::ostringstream txt;
    txt << "The non-existent code point 0x" << std::hex << c;
    GIVEN(txt.str()) {
      WHEN("It is encoded to UTF-16BE") {
        THEN("The coder must return an error") {
          std::string outString;
          const unicode::ParseResult r = unicode::Utf8Codec::appendChar(outString, c);
          CHECK(r == unicode::ParseResult::InvalidCharacter);
          CHECK(outString.empty());
        }
      }
    }
  }
}

TEST_CASE("Decode UTF-8, check decoding errors", "[unicode]")
{
  const std::vector<std::pair<std::string, unicode::ParseResult>>
      chars{std::make_pair("\xC1",                 unicode::ParseResult::IncompleteCharacter),
            std::make_pair("\xE1\x80",             unicode::ParseResult::IncompleteCharacter),
            std::make_pair("\xF1\x80\x80",         unicode::ParseResult::IncompleteCharacter),
            std::make_pair("\xC1\x20",             unicode::ParseResult::InvalidCharacter   ),
            std::make_pair("\xE1\x80\x20",         unicode::ParseResult::InvalidCharacter   ),
            std::make_pair("\xF1\x80\x80\x20",     unicode::ParseResult::InvalidCharacter   ),
            std::make_pair("\xC0\x8F",             unicode::ParseResult::InvalidCharacter   ), // overlong encoding
            std::make_pair("\xE0\x80\x81",         unicode::ParseResult::InvalidCharacter   ), // overlong encoding
            std::make_pair("\xF0\x80\x81\x80",     unicode::ParseResult::InvalidCharacter   ), // overlong encoding
            std::make_pair("\xF9\x80\x80\x80\x80", unicode::ParseResult::InvalidCharacter   ), // too long encoding
            std::make_pair("\xF4\x90\x80\x80",     unicode::ParseResult::InvalidCharacter   ), // too large number
            std::make_pair("\xF7\xBF\xBF\xBF",     unicode::ParseResult::InvalidCharacter   ), // too large number
            std::make_pair("\x81",                 unicode::ParseResult::InvalidCharacter   ), // unexpected continuation byte
            std::make_pair("\x80",                 unicode::ParseResult::InvalidCharacter   ), // unexpected continuation byte
            std::make_pair("\xED\xA0\x80",         unicode::ParseResult::InvalidCharacter   ), // start of invalid range
            std::make_pair("\xED\xBF\xBF",         unicode::ParseResult::InvalidCharacter   ), // end of invalid range
            std::make_pair("\xC1\xC1\x80",         unicode::ParseResult::InvalidCharacter   ),
            std::make_pair("\xE1\x80\xC1\x80",     unicode::ParseResult::InvalidCharacter   ),
            std::make_pair("\xF1\x80\x80\xC1\x80", unicode::ParseResult::InvalidCharacter   )
           };
  for(const std::pair<std::string, unicode::ParseResult> chr: chars) {
    std::ostringstream txt;
    txt << "The UTF-8 Sequence";
    for(char c: chr.first) { txt << " 0x" << std::hex << (static_cast<uint16_t>(c)&0xFF); }
    GIVEN(txt.str()) {
      WHEN("The unicode codepoint is decoded") {
        THEN("The decoder must return an error") {
          unicode::Utf32Char c;
          const unicode::ParseResult res = unicode::Utf8Codec::parseNext(c, chr.first.data(), chr.first.data()+chr.first.size());
          CHECK(res == chr.second);
        }
      }
    }
  }
}
