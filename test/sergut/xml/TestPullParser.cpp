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

#include "sergut/xml/PullParser.h"
#include "sergut/xml/detail/PullParserUtf16LE.h"
#include "sergut/xml/detail/PullParserUtf16BE.h"

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

std::string asciiToEncoding(const std::string& in, const TargetEncoding encoding, const bool addBOM = true) {
  const std::size_t outSize = encoding==TargetEncoding::Utf8 ? in.size() : in.size()*2+(addBOM?2:0);
  std::string out(outSize, '\0');
  // add the BOM
  std::string::iterator outIt = out.begin();
  if(addBOM) {
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
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty XML Root-Tag") {
        const std::string xml = asciiToEncoding("<root></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an almost empty XML Root-Tag") {
        const std::string xml = asciiToEncoding("<root> </root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty nested XML Tag") {
        const std::string xml = asciiToEncoding("<root><inner/></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty nested XML Tag with spaces") {
        const std::string xml = asciiToEncoding("<root> <inner/> </root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty nested XML Tag") {
        const std::string xml = asciiToEncoding("<root><inner></inner></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty nested XML Tag with spaces") {
        const std::string xml = asciiToEncoding("<root> <inner> </inner> </root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string(" "));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
    }
  }
}


TEST_CASE("XML-Parser (Invalid XML Test)", "[XML]")
{
  for(const TargetEncoding encodingType: encodings)
  {
    GIVEN("The " + toString(encodingType) + " PullParser") {
      WHEN("Parsing an XML with non-matching tags") {
        const std::string xml = asciiToEncoding("<root></beer>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Error);
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
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with a version XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml version=\"1.1\"?><root/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with an encoding XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml encoding=\"" + toString(encodingType, true) + "\"?><root/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with a version and encoding XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml version=\"1.1\" encoding=\"" + toString(encodingType, true) + "\"?><root/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with another version and encoding XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml  version='1.1'  \r\n\t encoding='" + toString(encodingType, true) + "'   \n\r\t ?><root/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The no error is detected by the parser") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an XML with an unsupported version XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml version=\"2.0\"?><root/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The parser returns an error") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Error);
        }
      }
      WHEN("Parsing an XML with an unsupported encoding XML-Declaration") {
        const std::string xml = asciiToEncoding("<?xml encoding=\"ISO-8859-15\"?><root/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The parser returns an error") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Error);
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
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty XML Root-Tag with Attributes and extra spaces") {
        const std::string xml = asciiToEncoding("<root   att1 = \"val1\"   att2   =   'val2'   />", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty XML Root-Tag with Attributes") {
        const std::string xml = asciiToEncoding("<root att1=\"val1\" att2='val2'></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty XML Root-Tag with other Attributes") {
        const std::string xml = asciiToEncoding("<root att1=\"val1\" att2='val2'></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty nested XML Tag with Attributes") {
        const std::string xml = asciiToEncoding("<root><inner att1=\"val1\" att2='val2'/></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an empty nested XML Tag with other Attributes") {
        const std::string xml = asciiToEncoding("<root><inner att1=\"val1\" att2='val2' /></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty nested XML Tag with Attributes") {
        const std::string xml = asciiToEncoding("<root><inner att1=\"val1\" att2='val2'></inner></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing another empty nested XML Tag with other Attributes") {
        const std::string xml = asciiToEncoding("<root><inner att1=\"val1\" att2='val2' ></inner></root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the same specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att1"));
          CHECK(parser.getCurrentValue() == std::string("val1"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("att2"));
          CHECK(parser.getCurrentValue() == std::string("val2"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("inner"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
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
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string("&"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity and Apostrophs in Text") {
        const std::string xml = asciiToEncoding("<root>'&apos;'</root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string("'''"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity and Quotes in Text") {
        const std::string xml = asciiToEncoding("<root>\"&apos;\"</root>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
          CHECK(parser.getCurrentValue() == std::string("\"'\""));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity in a Quote-Attribute") {
        const std::string xml = asciiToEncoding("<root attr=\"&quot;\"/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("attr"));
          CHECK(parser.getCurrentValue() == std::string("\""));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity in an Apostroph-Attribute") {
        const std::string xml = asciiToEncoding("<root attr='&apos;'/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("attr"));
          CHECK(parser.getCurrentValue() == std::string("'"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity with Apostrophs in a Quote-Attribute") {
        const std::string xml = asciiToEncoding("<root attr=\"'&quot;'\"/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("attr"));
          CHECK(parser.getCurrentValue() == std::string("'\"'"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
      WHEN("Parsing an Entity with Quotes in an Apostroph-Attribute") {
        const std::string xml = asciiToEncoding("<root attr='\"&apos;\"'/>", encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
        sergut::xml::PullParser& parser = *parserTmp;
        THEN("The result is the specified sequence") {
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
          CHECK(parser.getCurrentAttributeName() == std::string("attr"));
          CHECK(parser.getCurrentValue() == std::string("\"'\""));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
          CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseDocument);
        }
      }
    }
  }
}


TEST_CASE("XML-Parser (append data Test)", "[XML]")
{
  for(const TargetEncoding encodingType: encodings)
  {
    constexpr int REPETITION_COUNT=10;
    GIVEN("The " + toString(encodingType) + " PullParser") {
      WHEN("Starting to parse an incomplete XML and then completing the document after starting") {
        const std::string unencodedNested = "<nested attr=\"ad1\">content</nested>";
        const std::string initialXml = asciiToEncoding("<root>" + unencodedNested.substr(0, 10), encodingType);
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(initialXml));
        sergut::xml::PullParser& parser = *parserTmp;
        CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
        CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
        CHECK(parser.getCurrentTagName() == std::string("root"));
        CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
        CHECK(parser.getCurrentTagName() == std::string("nested"));
        const std::string remainingFragment = asciiToEncoding(unencodedNested.substr(10), encodingType, false);
        parser.appendData(remainingFragment.c_str(), remainingFragment.size());
        const std::string nested = asciiToEncoding(unencodedNested, encodingType, false);
        for(int i = 0; i < REPETITION_COUNT; ++i) {
          parser.appendData(nested.c_str(), nested.size());
        }
        const std::string closingTag = asciiToEncoding("</root>", encodingType, false);
        parser.appendData(closingTag.c_str(), closingTag.size());

        THEN("The no error is detected by the parser") {
          int i = 0;
          for(; i < REPETITION_COUNT + 1 && parser.isOk(); ++i) {
            CHECK(parser.getCurrentTokenType() == sergut::xml::ParseTokenType::OpenTag);
            CHECK(parser.getCurrentTagName() == std::string("nested"));
            CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Attribute);
            CHECK(parser.getCurrentAttributeName() == std::string("attr"));
            CHECK(parser.getCurrentValue() == std::string("ad1"));
            CHECK(parser.parseNext() == sergut::xml::ParseTokenType::Text);
            CHECK(parser.getCurrentValue() == std::string("content"));
            CHECK(parser.parseNext() == sergut::xml::ParseTokenType::CloseTag);
            CHECK(parser.getCurrentTagName() == std::string("nested"));
            parser.parseNext();
          }
          CHECK(i == REPETITION_COUNT + 1);
          CHECK(parser.getCurrentTokenType() == sergut::xml::ParseTokenType::CloseTag);
          CHECK(parser.getCurrentTagName() == std::string("root"));
        }
      }
    }
  }
}


static std::string getTextFromParser(const sergut::xml::PullParser& parser, const sergut::xml::ParseTokenType expectedTokenType)
{
  switch(expectedTokenType) {
  case sergut::xml::ParseTokenType::OpenTag:
  case sergut::xml::ParseTokenType::CloseTag:
    return parser.getCurrentTagName().toString();
  case sergut::xml::ParseTokenType::Attribute:
    return parser.getCurrentAttributeName().toString();
  case sergut::xml::ParseTokenType::Text:
    return parser.getCurrentValue().toString();
  case sergut::xml::ParseTokenType::InitialState:
  case sergut::xml::ParseTokenType::OpenDocument:
  case sergut::xml::ParseTokenType::CloseDocument:
  case sergut::xml::ParseTokenType::IncompleteDocument:
  case sergut::xml::ParseTokenType::Error:
    break;
  }
  return std::string();
}

TEST_CASE("XML-Parser (savepoint Test)", "[XML]")
{
  const std::vector<std::tuple<std::string, std::string, std::size_t>> testData{
    std::make_tuple( "<root><inner a", "tt=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner></root>",  3 ),
    std::make_tuple( "<root><inner att=\"1\"><", "v>1</v></inner><inner att=\"1\"><v>1</v></inner></root>",  4 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v>", "</inner><inner att=\"1\"><v>1</v></inner></root>",  7 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner", "><inner att=\"1\"><v>1</v></inner></root>",  7 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner>", "<inner att=\"1\"><v>1</v></inner></root>",  8 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner", " att=\"1\"><v>1</v></inner></root>",  8 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner a", "tt=\"1\"><v>1</v></inner></root>",  9 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\">", "<v>1</v></inner></root>", 10 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1", "</v></inner></root>", 12 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v>", "</inner></root>", 13 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner", "></root>", 13 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner>", "</root>", 14 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner></", "root>", 14 ),
    std::make_tuple( "<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner></root", ">", 14 )
  };
  const int savePoint1 = 2;
  const int savePoint2 = 8;
  const std::vector<std::tuple<sergut::xml::ParseTokenType, std::string>> expectedTokens{
    std::make_tuple( sergut::xml::ParseTokenType::OpenDocument,  std::string() ),
        std::make_tuple( sergut::xml::ParseTokenType::OpenTag,   std::string("root") ),
        std::make_tuple( sergut::xml::ParseTokenType::OpenTag,   std::string("inner") ),
        std::make_tuple( sergut::xml::ParseTokenType::Attribute, std::string("att") ),
        std::make_tuple( sergut::xml::ParseTokenType::OpenTag,   std::string("v") ),
        std::make_tuple( sergut::xml::ParseTokenType::Text,      std::string("1") ),
        std::make_tuple( sergut::xml::ParseTokenType::CloseTag,  std::string("v") ),
        std::make_tuple( sergut::xml::ParseTokenType::CloseTag,  std::string("inner") ),
        std::make_tuple( sergut::xml::ParseTokenType::OpenTag,   std::string("inner") ),
        std::make_tuple( sergut::xml::ParseTokenType::Attribute, std::string("att") ),
        std::make_tuple( sergut::xml::ParseTokenType::OpenTag,   std::string("v") ),
        std::make_tuple( sergut::xml::ParseTokenType::Text,      std::string("1") ),
        std::make_tuple( sergut::xml::ParseTokenType::CloseTag,  std::string("v") ),
        std::make_tuple( sergut::xml::ParseTokenType::CloseTag,  std::string("inner") ),
        std::make_tuple( sergut::xml::ParseTokenType::CloseTag,  std::string("root") ),
        std::make_tuple( sergut::xml::ParseTokenType::CloseDocument, std::string() )
  };
  for(const auto& values: testData) {
    const std::string& firstPart = std::get<0>(values);
    const std::string& secondPart = std::get<1>(values);
    const std::size_t interruptedAtCount = std::get<2>(values);
    for(const TargetEncoding encodingType: encodings)
    {
      GIVEN("The " + toString(encodingType) + " PullParser") {
        WHEN("Starting to parse an incomplete XML, setting a savePoint, Reading data out of the document until the end "
             "of the fragment is reached and then repeating the deserialization after the last save point")
        {
          const std::string  firstXmlFragment = asciiToEncoding( firstPart, encodingType);
          const std::string secondXmlFragment = asciiToEncoding(secondPart, encodingType, false);

          std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(firstXmlFragment));
          sergut::xml::PullParser& parser = *parserTmp;

          THEN("The parser can restart at the savepoint and correctly proceed the parsing") {
            std::size_t currentSavepoint = 0;
            for(std::size_t i = 0; i < interruptedAtCount; ++i) {
              CHECK(parser.parseNext() == std::get<0>(expectedTokens[i]));
              CHECK(getTextFromParser(parser, std::get<0>(expectedTokens[i])) == std::get<1>(expectedTokens[i]));
              if(i == savePoint1 || i == savePoint2) {
                CHECK(parser.setSavePoint() == true);
                currentSavepoint = i;
              }
            }
            CHECK(parser.parseNext() == sergut::xml::ParseTokenType::IncompleteDocument);

            if(currentSavepoint%2==0) {
              // check that restoreToSavePoint() can be called before appendData()
              CHECK(parser.restoreToSavePoint() == true);
              parser.appendData(secondXmlFragment.data(), secondXmlFragment.size());
            } else {
              // check that appendData() can be called before restoreToSavePoint()
              parser.appendData(secondXmlFragment.data(), secondXmlFragment.size());
              CHECK(parser.restoreToSavePoint() == true);
            }

            CHECK(parser.getCurrentTokenType() == std::get<0>(expectedTokens[currentSavepoint]));
            CHECK(getTextFromParser(parser, std::get<0>(expectedTokens[currentSavepoint]))
                  == std::get<1>(expectedTokens[currentSavepoint]));

            for(std::size_t i = currentSavepoint+1; i < expectedTokens.size(); ++i) {
              CHECK(parser.parseNext() == std::get<0>(expectedTokens[i]));
              CHECK(getTextFromParser(parser, std::get<0>(expectedTokens[i])) == std::get<1>(expectedTokens[i]));
            }
          }
        }
      }
    }
  }
}
