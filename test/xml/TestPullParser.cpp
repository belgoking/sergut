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

#include "xml/PullParser.h"
#include "xml/detail/PullParserNVUtf16LE.h"
#include "xml/detail/PullParserNVUtf16BE.h"

enum class TargetEncoding { Utf8, Utf16BE, Utf16LE };
std::string toString(const TargetEncoding encoding, const bool shortDesc = false)
{
  switch (encoding) {
  case TargetEncoding::Utf8:    return "UTF-8";
  case TargetEncoding::Utf16BE: return shortDesc ? "UTF-16" : "UTF-16BE";
  case TargetEncoding::Utf16LE: return shortDesc ? "UTF-16" : "UTF-16LE";
  }
  return "";
}

static const std::vector<TargetEncoding> encodings{ TargetEncoding::Utf8, TargetEncoding::Utf16BE, TargetEncoding::Utf16LE };

std::string asciiToEncoding(const std::string& in, const TargetEncoding encoding) {
  std::string out(encoding==TargetEncoding::Utf8 ? in.size() : in.size()*2+2, '\0');
  // add the BOM
  std::string::iterator outIt = out.begin();
  switch (encoding) {
  case TargetEncoding::Utf16BE:
    *(outIt++) = 0xFE;
    *(outIt++) = 0xFF;
    break;
  case TargetEncoding::Utf16LE:
    *(outIt++) = 0xFF;
    *(outIt++) = 0xFE;
    break;
  case TargetEncoding::Utf8:
    // no BOM
    break;
  }
  std::string::const_iterator inIt = in.begin();
  while(inIt != in.end()) {
    if(encoding == TargetEncoding::Utf16BE) {
      *(outIt++) = '\0';
    }
    *(outIt++) = *(inIt++);
    if(encoding == TargetEncoding::Utf16LE) {
      *(outIt++) = '\0';
    }
  }
  return out;
}

TEST_CASE("XML-Parser (Simple Tests)", "[XML]")
{
  for(const TargetEncoding encodingType: encodings)
  {
    GIVEN("The " + toString(encodingType) + " PullParser") {
      WHEN("Parsing an empty XML Root-Tag") {
        const std::string xml = asciiToEncoding("<root/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty XML Root-Tag") {
        const std::string xml = asciiToEncoding("<root></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an almost empty XML Root-Tag") {
        const std::string xml = asciiToEncoding("<root> </root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty nested XML Tag") {
        const std::string xml = asciiToEncoding("<root><inner/></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty nested XML Tag with spaces") {
        const std::string xml = asciiToEncoding("<root> <inner/> </root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty nested XML Tag") {
        const std::string xml = asciiToEncoding("<root><inner></inner></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty nested XML Tag with spaces") {
        const std::string xml = asciiToEncoding("<root> <inner> </inner> </root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
    }
  }
}


TEST_CASE("XML-Parser (Don't check Tags Test)", "[XML]")
{
  for(const TargetEncoding encodingType: encodings)
  {
    GIVEN("The " + toString(encodingType) + " PullParser") {
      WHEN("Parsing an XML with non-matching tags") {
        const std::string xml = asciiToEncoding("<root></beer>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("beer"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
    }
  }
}


TEST_CASE("XML-Parser (Check XML-Declaration Test)", "[XML]")
{
  for(const TargetEncoding encodingType: encodings)
  {
    GIVEN("The " + toString(encodingType) + " PullParser") {
      WHEN("Parsing an XML with an empty XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml?><root/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with a version XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml version=\"1.1\"?><root/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with an encoding XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml encoding=\"" + toString(encodingType, true) + "\"?><root/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with a version and encoding XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml version=\"1.1\" encoding=\"" + toString(encodingType, true) + "\"?><root/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with another version and encoding XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml  version='1.1'  \r\n\t encoding='" + toString(encodingType, true) + "'   \n\r\t ?><root/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with an unsupported version XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml version=\"2.0\"?><root/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The parser returns an error") {
          CHECK(parser.parseNext() == xml::ParseTokenType::Error);
        }
      }
      WHEN("Parsing an XML with an unsupported encoding XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml encoding=\"ISO-8859-15\"?><root/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The parser returns an error") {
          CHECK(parser.parseNext() == xml::ParseTokenType::Error);
        }
      }
    }
  }
}


TEST_CASE("XML-Parser (Attribute Tests)", "[XML]")
{
  for(const TargetEncoding encodingType: encodings)
  {
    GIVEN("The " + toString(encodingType) + " PullParser") {
      WHEN("Parsing an empty XML Root-Tag with Attributes") {
        const std::string xml = asciiToEncoding("<root att1=\"val1\" att2='val2'/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty XML Root-Tag with Attributes and extra spaces") {
        const std::string xml = asciiToEncoding("<root   att1 = \"val1\"   att2   =   'val2'   />", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty XML Root-Tag with Attributes") {
        const std::string xml = asciiToEncoding("<root att1=\"val1\" att2='val2'></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty XML Root-Tag with other Attributes") {
        const std::string xml = asciiToEncoding("<root att1=\"val1\" att2='val2'></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty nested XML Tag with Attributes") {
        const std::string xml = asciiToEncoding("<root><inner att1=\"val1\" att2='val2'/></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty nested XML Tag with other Attributes") {
        const std::string xml = asciiToEncoding("<root><inner att1=\"val1\" att2='val2' /></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty nested XML Tag with Attributes") {
        const std::string xml = asciiToEncoding("<root><inner att1=\"val1\" att2='val2'></inner></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty nested XML Tag with other Attributes") {
        const std::string xml = asciiToEncoding("<root><inner att1=\"val1\" att2='val2' ></inner></root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
    }
  }
}


TEST_CASE("XML-Parser (Entity Test)", "[XML]")
{
  for(const TargetEncoding encodingType: encodings)
  {
    GIVEN("The " + toString(encodingType) + " PullParser") {
      WHEN("Parsing an Entity in Text") {
        const std::string xml = asciiToEncoding("<root>&amp;</root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string("&"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity and Apostrophs in Text") {
        const std::string xml = asciiToEncoding("<root>'&apos;'</root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string("'''"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity and Quotes in Text") {
        const std::string xml = asciiToEncoding("<root>\"&apos;\"</root>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string("\"'\""));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity in a Quote-Attribute") {
        const std::string xml = asciiToEncoding("<root attr=\"&quot;\"/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("attr"));
          CHECK(parser.getCurrentValue() == std::string("\""));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity in an Apostroph-Attribute") {
        const std::string xml = asciiToEncoding("<root attr='&apos;'/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("attr"));
          CHECK(parser.getCurrentValue() == std::string("'"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity with Apostrophs in a Quote-Attribute") {
        const std::string xml = asciiToEncoding("<root attr=\"'&quot;'\"/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("attr"));
          CHECK(parser.getCurrentValue() == std::string("'\"'"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity with Quotes in an Apostroph-Attribute") {
        const std::string xml = asciiToEncoding("<root attr='\"&apos;\"'/>", encodingType);
        std::unique_ptr<xml::PullParser> parserTmp = xml::PullParser::createParser(misc::StringRef(xml));
        xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("attr"));
          CHECK(parser.getCurrentValue() == std::string("\"'\""));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == xml::ParseTokenType::CloseDocument);
        }
      }
    }
  }
}
