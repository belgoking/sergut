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

#include "unicode/Utf16Codec.h"

#include <sstream>
#include <iomanip>
#include <tuple>

TEST_CASE("Code/Decode UTF-16BE", "[unicode]")
{
  const std::vector<std::tuple<unicode::Utf32Char, std::string, std::string>>
      chars{
            std::make_tuple(0x00,     std::string(2, '\0'),               std::string(2, '\0')              ),
            std::make_tuple(0x24,     std::string("\x00\x24", 2),         std::string("\x24\x00", 2)        ),
            std::make_tuple(0x7F,     std::string("\x00\x7F", 2),         std::string("\x7F\x00", 2)        ),
            std::make_tuple(0x20AC,   std::string("\x20\xAC", 2),         std::string("\xAC\x20", 2)        ),
            std::make_tuple(0xFFFF,   std::string("\xFF\xFF", 2),         std::string("\xFF\xFF", 2)        ),
            std::make_tuple(0x10000,  std::string("\xD8\x00\xDC\x00", 4), std::string("\x00\xD8\x00\xDC", 4)),
            std::make_tuple(0x10437,  std::string("\xD8\x01\xDC\x37", 4), std::string("\x01\xD8\x37\xDC", 4)),
            std::make_tuple(0x24B62,  std::string("\xD8\x52\xDF\x62", 4), std::string("\x52\xD8\x62\xDF", 4)),
            std::make_tuple(0x10FFFF, std::string("\xDB\xFF\xDF\xFF", 4), std::string("\xFF\xDB\xFF\xDF", 4))};
  for(const std::tuple<unicode::Utf32Char, std::string, std::string> chr: chars) {
    const unicode::Utf32Char utf32Char = std::get<0>(chr);
    const std::string& utf16BE = std::get<1>(chr);
    const std::string& utf16LE = std::get<2>(chr);
    {
      std::ostringstream txt;
      txt << "The unicode Character 0x" << std::hex << utf32Char;
      GIVEN(txt.str()) {
        WHEN("The character is encoded to UTF-16BE") {
          std::string out;
          unicode::Utf16BECodec::appendChar(out, utf32Char);
          THEN("The result is the specified sequence") {
            CHECK(out == utf16BE);
          }
        }
        WHEN("The character is encoded to UTF-16LE") {
          std::string out;
          unicode::Utf16LECodec::appendChar(out, utf32Char);
          THEN("The result is the specified sequence") {
            CHECK(out == utf16LE);
          }
        }
      }
    }
    {
      std::ostringstream txt;
      txt << "The UTF-16BE Sequence";
      for(char c: utf16BE) { txt << " 0x" << std::hex << (static_cast<uint16_t>(c)&0xFF); }
      GIVEN(txt.str()) {
        WHEN("The unicode codepoint is decoded") {
          unicode::Utf32Char c;
          const unicode::ParseResult res = unicode::Utf16BECodec::parseNext(c, utf16BE.data(), utf16BE.data()+utf16BE.size());
          THEN("The result is the specified value") {
            CHECK(res == unicode::ParseResult(utf16BE.size()));
            CHECK(c == utf32Char);
          }
        }
      }
    }
    {
      std::ostringstream txt;
      txt << "The UTF-16LE Sequence";
      for(char c: utf16LE) { txt << " 0x" << std::hex << (static_cast<uint16_t>(c)&0xFF); }
      GIVEN(txt.str()) {
        WHEN("The unicode codepoint is decoded") {
          unicode::Utf32Char c;
          const unicode::ParseResult res = unicode::Utf16LECodec::parseNext(c, utf16LE.data(), utf16LE.data()+utf16LE.size());
          THEN("The result is the specified value") {
            CHECK(res == unicode::ParseResult(utf16LE.size()));
            CHECK(c == utf32Char);
          }
        }
      }
    }
  }
}

TEST_CASE("Code UTF-16BE, check encoding errors", "[unicode]")
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
          const unicode::ParseResult r = unicode::Utf16BECodec::appendChar(outString, c);
          CHECK(r == unicode::ParseResult::InvalidCharacter);
          CHECK(outString.empty());
        }
      }
      WHEN("It is encoded to UTF-16LE") {
        THEN("The coder must return an error") {
          std::string outString;
          const unicode::ParseResult r = unicode::Utf16LECodec::appendChar(outString, c);
          CHECK(r == unicode::ParseResult::InvalidCharacter);
          CHECK(outString.empty());
        }
      }
    }
  }
}

TEST_CASE("Decode UTF-16BE, check decoding errors", "[unicode]")
{
  const std::vector<std::tuple<std::string, std::string, unicode::ParseResult>>
      chars{std::make_tuple("\x20",             "\x20",             unicode::ParseResult::IncompleteCharacter),
            std::make_tuple("\xD8\x20",         "\x20\xD8",         unicode::ParseResult::IncompleteCharacter),
            std::make_tuple("\xD8\x20\xDC",     "\x20\xD8\x20",     unicode::ParseResult::IncompleteCharacter),
            std::make_tuple("\xDC\x20",         "\x20\xDC",         unicode::ParseResult::InvalidCharacter   ),
            std::make_tuple("\xD8\x80\x80\x20", "\x80\xD8\x20\x80", unicode::ParseResult::InvalidCharacter   ),
            std::make_tuple("\xD8\x80\xD8\x20", "\x80\xD8\x20\xD8", unicode::ParseResult::InvalidCharacter   ),
            std::make_tuple("\xD8\x80\x80\x20", "\x80\xD8\x20\x80", unicode::ParseResult::InvalidCharacter   ),
           };
  for(const std::tuple<std::string, std::string, unicode::ParseResult> chr: chars) {
    const std::string&         utf16BE     = std::get<0>(chr);
    const std::string&         utf16LE     = std::get<1>(chr);
    const unicode::ParseResult parseResult = std::get<2>(chr);
    {
      std::ostringstream txt;
      txt << "The UTF-16BE Sequence";
      for(char c: utf16BE) { txt << " 0x" << std::hex << (static_cast<uint16_t>(c)&0xFF); }
      GIVEN(txt.str()) {
        WHEN("The unicode codepoint is decoded") {
          THEN("The decoder must return an error") {
            unicode::Utf32Char c;
            const unicode::ParseResult res = unicode::Utf16BECodec::parseNext(c, utf16BE.data(), utf16BE.data()+utf16BE.size());
            CHECK(res == parseResult);
          }
        }
      }
    }
    {
      std::ostringstream txt;
      txt << "The UTF-16LE Sequence";
      for(char c: utf16LE) { txt << " 0x" << std::hex << (static_cast<uint16_t>(c)&0xFF); }
      GIVEN(txt.str()) {
        WHEN("The unicode codepoint is decoded") {
          THEN("The decoder must return an error") {
            unicode::Utf32Char c;
            const unicode::ParseResult res = unicode::Utf16LECodec::parseNext(c, utf16LE.data(), utf16LE.data()+utf16LE.size());
            CHECK(res == parseResult);
          }
        }
      }
    }
  }
}
