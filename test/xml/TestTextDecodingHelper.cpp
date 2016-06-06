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

#include "xml/detail/TextDecodingHelper.h"

#include <tuple>

typedef xml::detail::TextDecodingHelper<unicode::Utf8Codec> Utf8DecodingHelper;

TEST_CASE("XML-Parser UTF-8 (EntityHandling - OK)", "[XML]")
{
  const std::vector<std::pair<std::string, std::string>>
      chars{std::make_pair("", ""),
        std::make_pair("&lt;",   "<"),
        std::make_pair("&amp;",  "&"),
        std::make_pair("&apos;", "'"),
        std::make_pair("&quot;", "\""),
        std::make_pair("&#45;",  "-"),
        std::make_pair("&#x2D;", "-"),
        std::make_pair("&#x2d;", "-"),
        std::make_pair("-&apos;", "-'"),
        std::make_pair("&apos;-", "'-"),
        std::make_pair(" &apos; ", " ' "),
  };
  for(const std::pair<std::string, std::string>& chr: chars) {
    GIVEN("The Character entity '" + chr.first + "'") {

      const std::vector<std::pair<Utf8DecodingHelper::TextType, std::string>>
          textTypesNTerminatingChars{
            {Utf8DecodingHelper::TextType::Plain,         ""  },
            {Utf8DecodingHelper::TextType::AttValueApos,  "'" },
            {Utf8DecodingHelper::TextType::AttValueQuote, "\""},
            {Utf8DecodingHelper::TextType::CharData,      "<" },
          };
      for(const std::pair<Utf8DecodingHelper::TextType, std::string>& tt: textTypesNTerminatingChars) {
        WHEN("Parsing using the TextDecodingHelper in TextType=" + std::to_string(int(tt.first))) {
          THEN("The result is the specified sequence") {
            const std::string in = chr.first + tt.second;
            std::string out(chr.second.size(), '\0');
            Utf8DecodingHelper helper(&*out.begin(), &*out.end(), tt.first,
                                           &*in.begin(), &*in.end());
            CHECK(helper.decodeText());
            CHECK(helper.getWriteCount() == chr.second.size());
            CHECK(out == chr.second);
          }
        }
      }
    }
  }
}

TEST_CASE("XML-Parser UTF-8 (EntityHandling - error)", "[XML]")
{
  const std::vector<std::string>
      chars{
            "&nok;", // unknown entity
            "&#;",
            "&#a;",
            "&#g;",
            "&#1a;",
            "&#1g;",
            "&#lg;",
            "&#65534;", // illegal char
            "&#65535;", // illegal char
            "&#1114112;", // too large
            "&#11141111;", // too large
            "&#11141111", // too large
            "&#-20;", // negative number
            "&#x;",
            "&#x1g;",
            "&#xlg;",
            "&#xg;",
            "&#xfffe;", // illegal char
            "&#xffff;", // illegal char
            "&#x110000;", // too large
            "&#xffffffff;", // too large
            "&#xffffffff", // too large
            "&#x-2D;", // negative number
            " &nok;", // unknown entity
            " &#;",
            " &#a;",
            " &#g;",
            " &#1a;",
            " &#1g;",
            " &#lg;",
            " &#65534;", // illegal char
            " &#65535;", // illegal char
            " &#1114112;", // too large
            " &#11141111;", // too large
            " &#11141111", // too large
            " &#-20;", // negative number
            " &#x;",
            " &#x1g;",
            " &#xlg;",
            " &#xg;",
            " &#xfffe;", // illegal char
            " &#xffff;", // illegal char
            " &#x110000;", // too large
            " &#xffffffff;", // too large
            " &#xffffffff", // too large
            " &#x-2D;", // negative number
            "&nok; ", // unknown entity
            "&#; ",
            "&#a; ",
            "&#g; ",
            "&#1a; ",
            "&#1g; ",
            "&#lg; ",
            "&#65534; ", // illegal char
            "&#65535; ", // illegal char
            "&#1114112; ", // too large
            "&#11141111; ", // too large
            "&#11141111 ", // too large
            "&#-20; ", // negative number
            "&#x; ",
            "&#x1g; ",
            "&#xlg; ",
            "&#xg; ",
            "&#xfffe; ", // illegal char
            "&#xffff; ", // illegal char
            "&#x110000; ", // too large
            "&#xffffffff; ", // too large
            "&#xffffffff ", // too large
            "&#x-2D; ", // negative number
           };
  for(const std::string& chr: chars) {
    GIVEN("The illegal Character entity '" + chr +"'") {
      const std::vector<std::pair<Utf8DecodingHelper::TextType, std::string>>
          textTypesNTerminatingChars{
            {Utf8DecodingHelper::TextType::Plain,         ""  },
            {Utf8DecodingHelper::TextType::AttValueApos,  "'" },
            {Utf8DecodingHelper::TextType::AttValueQuote, "\""},
            {Utf8DecodingHelper::TextType::CharData,      "<" },
          };
      for(const std::pair<Utf8DecodingHelper::TextType, std::string>& tt: textTypesNTerminatingChars) {
        WHEN("Parsing using the TextDecodingHelper in TextType=" + std::to_string(int(tt.first))) {
          THEN("A parsing error occurs") {
            const std::string in = chr + tt.second;
            std::string out(13, '\0');
            Utf8DecodingHelper helper(&*out.begin(), &*out.end(), tt.first,
                                           &*in.begin(), &*in.end());
            CHECK(!helper.decodeText());
            CHECK(helper.isError());
            CHECK(!helper.isIncomplete());
          }
        }
      }
    }
  }
}

TEST_CASE("XML-Parser UTF-8 (EntityHandling - incomplete)", "[XML]")
{
  const std::vector<std::string>
      chars{"&lt",
            "&#",
            "&#1",
            "&#12",
            "&#x",
            "&#x1a",
            " \xf4\x8f", // ending on incomplete UTF-8 character
           };
  for(const std::string& chr: chars) {
    GIVEN("The incomplete Character entity/String '" + chr +"'") {
      const std::vector<Utf8DecodingHelper::TextType>
          textTypesNTerminatingChars{
            Utf8DecodingHelper::TextType::Plain,
            Utf8DecodingHelper::TextType::AttValueApos,
            Utf8DecodingHelper::TextType::AttValueQuote,
            Utf8DecodingHelper::TextType::CharData,
          };
      for(const Utf8DecodingHelper::TextType tt: textTypesNTerminatingChars) {
        WHEN("Parsing using the TextDecodingHelper in TextType=" + std::to_string(int(tt))) {
          THEN("The result is the specified sequence") {
            std::string out(13, '\0');
            Utf8DecodingHelper helper(&*out.begin(), &*out.end(), tt,
                                           &*chr.begin(), &*chr.end());
            CHECK(!helper.decodeText());
            CHECK(!helper.isError());
            CHECK(helper.isIncomplete());
          }
        }
      }
    }
  }
}

TEST_CASE("XML-Parser UTF-8 (Illegal character Handling)", "[XML]")
{
  const std::vector<std::pair<Utf8DecodingHelper::TextType, std::string>>
      textTypesNStrings{
        {Utf8DecodingHelper::TextType::AttValueApos,  "<"  },
        {Utf8DecodingHelper::TextType::AttValueApos,  " <" },
        {Utf8DecodingHelper::TextType::AttValueApos,  "< " },
        {Utf8DecodingHelper::TextType::AttValueApos,  " < "},
        {Utf8DecodingHelper::TextType::AttValueQuote, "<"  },
        {Utf8DecodingHelper::TextType::AttValueQuote, " <" },
        {Utf8DecodingHelper::TextType::AttValueQuote, "< " },
        {Utf8DecodingHelper::TextType::AttValueQuote, " < "},
        {Utf8DecodingHelper::TextType::CharData,      "<"  },
        {Utf8DecodingHelper::TextType::CharData,      " <" },
        {Utf8DecodingHelper::TextType::CharData,      "< " },
        {Utf8DecodingHelper::TextType::CharData,      " < "},
      };
  for(const std::pair<Utf8DecodingHelper::TextType, std::string>& tt: textTypesNStrings) {
    GIVEN("The illegal Character in String '" + tt.second +"'") {
      WHEN("Parsing using the TextDecodingHelper in TextType=" + std::to_string(int(tt.first))) {
        THEN("The result is the specified sequence") {
          std::string out(13, '\0');
          Utf8DecodingHelper helper(&*out.begin(), &*out.end(), tt.first,
                                         &*tt.second.begin(), &*tt.second.end());
          CHECK(!helper.decodeText());
          CHECK(helper.isError());
          CHECK(!helper.isIncomplete());
        }
      }
    }
  }
}

TEST_CASE("XML-Parser UTF-8 (Legal character Handling)", "[XML]")
{
  const std::vector<std::tuple<Utf8DecodingHelper::TextType, std::string, std::string>>
      textTypesNStrings{
        std::make_tuple(Utf8DecodingHelper::TextType::Plain,         "\"",    "\""  ),
        std::make_tuple(Utf8DecodingHelper::TextType::Plain,         " \"",   " \"" ),
        std::make_tuple(Utf8DecodingHelper::TextType::Plain,         "\" ",   "\" " ),
        std::make_tuple(Utf8DecodingHelper::TextType::Plain,         " \" ",  " \" "),
        std::make_tuple(Utf8DecodingHelper::TextType::Plain,         "'",     "'"   ),
        std::make_tuple(Utf8DecodingHelper::TextType::Plain,         " '",    " '"  ),
        std::make_tuple(Utf8DecodingHelper::TextType::Plain,         "' ",    "' "  ),
        std::make_tuple(Utf8DecodingHelper::TextType::Plain,         " ' ",   " ' " ),
        std::make_tuple(Utf8DecodingHelper::TextType::AttValueApos,  "\"'",   "\""  ),
        std::make_tuple(Utf8DecodingHelper::TextType::AttValueApos,  " \"'",  " \"" ),
        std::make_tuple(Utf8DecodingHelper::TextType::AttValueApos,  "\" '",  "\" " ),
        std::make_tuple(Utf8DecodingHelper::TextType::AttValueApos,  " \" '", " \" "),
        std::make_tuple(Utf8DecodingHelper::TextType::AttValueQuote, "'\"",   "'"   ),
        std::make_tuple(Utf8DecodingHelper::TextType::AttValueQuote, " '\"",  " '"  ),
        std::make_tuple(Utf8DecodingHelper::TextType::AttValueQuote, "' \"",  "' "  ),
        std::make_tuple(Utf8DecodingHelper::TextType::AttValueQuote, " ' \"", " ' " ),
        std::make_tuple(Utf8DecodingHelper::TextType::CharData,      "\"<",   "\""  ),
        std::make_tuple(Utf8DecodingHelper::TextType::CharData,      " \"<",  " \"" ),
        std::make_tuple(Utf8DecodingHelper::TextType::CharData,      "\" <",  "\" " ),
        std::make_tuple(Utf8DecodingHelper::TextType::CharData,      " \" <", " \" "),
        std::make_tuple(Utf8DecodingHelper::TextType::CharData,      "'<",    "'"   ),
        std::make_tuple(Utf8DecodingHelper::TextType::CharData,      " '<",   " '"  ),
        std::make_tuple(Utf8DecodingHelper::TextType::CharData,      "' <",   "' "  ),
        std::make_tuple(Utf8DecodingHelper::TextType::CharData,      " ' <",  " ' " ),
      };
  for(const std::tuple<Utf8DecodingHelper::TextType, std::string, std::string>& tt: textTypesNStrings) {
    GIVEN("The legal Characters in String '" + std::get<1>(tt) +"'") {
      WHEN("Parsing using the TextDecodingHelper in TextType=" + std::to_string(int(std::get<0>(tt)))) {
        THEN("The result is the specified sequence") {
          std::string out(std::get<1>(tt).size(), '\0');
          Utf8DecodingHelper helper(&*out.begin(), &*out.end(), std::get<0>(tt),
                                         &*std::get<1>(tt).begin(), &*std::get<1>(tt).end());
          CHECK(helper.decodeText());
          CHECK(helper.getWriteCount() == std::get<2>(tt).size());
          out.resize(helper.getWriteCount());
          CHECK(out == std::get<2>(tt));
          CHECK(!helper.isError());
          CHECK(!helper.isIncomplete());
        }
      }
    }
  }
}
