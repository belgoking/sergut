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

#include <catch2/catch.hpp>

#include "TestSupportClasses.h"

#include "sergut/XmlDeserializer.h"
#include "sergut/XmlDeserializerTiny.h"
#include "sergut/XmlDeserializerTiny2.h"
#include "sergut/XmlSerializer.h"
#include "sergut/SerializerBase.h"
#include "sergut/DeserializerBase.h"

#include <cctype>
#include <cinttypes>
#include <iostream>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// check error handling
////////////////////////////////////////////////////////////////////////////////
// this is a prequel to the checks for invalid XML
TEST_CASE("Deserialize valid XML", "[sergut]")
{
  SECTION("OK-Test") {
    sergut::XmlDeserializer deser("<rootTag><mandatoryMember>10</mandatoryMember><optionalMember>23</optionalMember></rootTag>");
    const SomeTestData td = deser.deserializeData<SomeTestData>("rootTag");
    CHECK(td == (SomeTestData{10, 23}));
  }
  SECTION("OK-Test with missing optional") {
    sergut::XmlDeserializer deser("<rootTag><mandatoryMember>10</mandatoryMember></rootTag>");
    const SomeTestData td = deser.deserializeData<SomeTestData>("rootTag");
    CHECK(td == (SomeTestData{10, 0}));
  }
  SECTION("OK-Test") {
    sergut::XmlDeserializer deser("<rootTag><mandatoryMember>10</mandatoryMember><optionalMember>23</optionalMember></rootTag>");
    const SomeTestData td = deser.deserializeData<SomeTestData>(nullptr);
    CHECK(td == (SomeTestData{10, 23}));
  }
}

TEST_CASE("Deserialize invalid XML", "[sergut]")
{
  SECTION("Wrong root tag") {
    sergut::XmlDeserializer deser("<wrongTag1><someMember>10</someMember></wrongTag1>");
    CHECK_THROWS_AS(deser.deserializeData<SomeTestData>("otherRootTag"), sergut::ParsingException);
  }
  SECTION("Mandatory member missing") {
    sergut::XmlDeserializer deser("<rootTag><someWrongMember>10</someWrongMember></rootTag>");
    CHECK_THROWS_AS(deser.deserializeData<SomeTestData>("rootTag"), sergut::ParsingException);
  }
}

////////////////////////////////////////////////////////////////////////////////
// check bool
////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Deserialize bool", "[sergut]")
{
  const std::vector<std::pair<std::string, bool>> data{
    {"true", true}, {"1", true}, {"t", true}, {"T", true}, {"True", true}, {"TRUE", true}, {"2", true},
    {"false", false}, {"0", false}, {"f", false}, {"F", false}, {"False", false}, {"FALSE", false}
  };
  for(const std::pair<std::string, bool> d: data) {
    const std::string xml = "<bool>" + d.first + "</bool>";
    GIVEN(("The string '" + xml + "'").c_str()) {
      WHEN("The string is decoded with XmlDeserializer") {
        sergut::XmlDeserializer deser(xml);
        const bool b = deser.deserializeData<bool>("bool");
        THEN("The result is the specified sequence") {
          CHECK(b == d.second);
        }
      }
      WHEN("The string is decoded with XmlDeserializerTiny") {
        sergut::XmlDeserializerTiny deser(xml);
        const bool b = deser.deserializeData<bool>("bool");
        THEN("The result is the specified sequence") {
          CHECK(b == d.second);
        }
      }
      WHEN("The string is decoded with XmlDeserializerTiny2") {
        sergut::XmlDeserializerTiny2 deser(xml);
        const bool b = deser.deserializeData<bool>("bool");
        THEN("The result is the specified sequence") {
          CHECK(b == d.second);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Test serialization/deserialization of Data
////////////////////////////////////////////////////////////////////////////////

#define DEFINE_SIMPLE_DATATYPE_TEST(type, datatypeValue, expectedResult) \
struct Ser_ ## type { \
  type value; \
  bool operator==(const Ser_ ## type& rhs) const { return value == rhs.value; } \
}; \
SERGUT_FUNCTION(Ser_ ## type, data, ar) \
{ \
  ar & SERGUT_MMEMBER(data, value); \
} \
TEST_CASE("DeSerialize datatype " #type " as XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    Ser_ ## type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData(#type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const Ser_ ## type tpDeser1 = deser.deserializeData<Ser_ ## type>(#type); \
    CHECK(tpDeser1 == Ser_ ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
  sergut::XmlDeserializerTiny deser(expectedResult); \
  const Ser_ ## type tpDeser2 = deser.deserializeData<Ser_ ## type>(#type); \
  CHECK(tpDeser2 == Ser_ ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
  sergut::XmlDeserializerTiny2 deser(expectedResult); \
  const Ser_ ## type tpDeser2 = deser.deserializeData<Ser_ ## type>(#type); \
  CHECK(tpDeser2 == Ser_ ## type{ datatypeValue }); \
  } \
} \

DEFINE_SIMPLE_DATATYPE_TEST(bool,       true, "<bool value=\"true\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(char,        'a', "<char value=\"a\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(uint8_t,      19, "<uint8_t value=\"19\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(int16_t,  -32767, "<int16_t value=\"-32767\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(uint16_t,  65535, "<uint16_t value=\"65535\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(int32_t,  -32768, "<int32_t value=\"-32768\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(uint32_t,  65536, "<uint32_t value=\"65536\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(int64_t,  -32768, "<int64_t value=\"-32768\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(uint64_t,  65536, "<uint64_t value=\"65536\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(float,      2.25, "<float value=\"2.25\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(double,     2.25, "<double value=\"2.25\"/>")
DEFINE_SIMPLE_DATATYPE_TEST(Time,     (Time{14, 34, 15}), "<Time value=\"14:34:15\"/>")
namespace {
typedef std::string string;
DEFINE_SIMPLE_DATATYPE_TEST(string, "hallo Du", "<string value=\"hallo Du\"/>")
}
namespace {
typedef std::string escapedString;
DEFINE_SIMPLE_DATATYPE_TEST(escapedString, "<b>\"STRING&amp; STRONG\"</b>", "<escapedString value=\"&lt;b&gt;&quot;STRING&amp;amp; STRONG&quot;&lt;/b&gt;\"/>")
}


#define DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(type, datatypeValue, expectedResult) \
struct SerAsChild_ ## type { \
  type value; \
  bool operator==(const SerAsChild_ ## type& rhs) const { return value == rhs.value; } \
}; \
SERGUT_FUNCTION(SerAsChild_ ## type, data, ar) \
{ \
  ar & sergut::children & SERGUT_MMEMBER(data, value); \
} \
TEST_CASE("DeSerialize datatype " #type " as child in XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    SerAsChild_ ## type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData(#type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const SerAsChild_ ## type tpDeser1 = deser.deserializeData<SerAsChild_ ## type>(#type); \
    CHECK(tpDeser1 == SerAsChild_ ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
  sergut::XmlDeserializerTiny deser(expectedResult); \
  const SerAsChild_ ## type tpDeser2 = deser.deserializeData<SerAsChild_ ## type>(#type); \
  CHECK(tpDeser2 == SerAsChild_ ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
  sergut::XmlDeserializerTiny2 deser(expectedResult); \
  const SerAsChild_ ## type tpDeser2 = deser.deserializeData<SerAsChild_ ## type>(#type); \
  CHECK(tpDeser2 == SerAsChild_ ## type{ datatypeValue }); \
  } \
} \

DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(bool,      false, "<bool><value>false</value></bool>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(char,        'a', "<char><value>a</value></char>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(uint8_t,      19, "<uint8_t><value>19</value></uint8_t>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(int16_t,  -32767, "<int16_t><value>-32767</value></int16_t>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(uint16_t,  65535, "<uint16_t><value>65535</value></uint16_t>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(int32_t,  -32768, "<int32_t><value>-32768</value></int32_t>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(uint32_t,  65536, "<uint32_t><value>65536</value></uint32_t>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(int64_t,  -32768, "<int64_t><value>-32768</value></int64_t>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(uint64_t,  65536, "<uint64_t><value>65536</value></uint64_t>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(float,      2.25, "<float><value>2.25</value></float>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(double,     2.25, "<double><value>2.25</value></double>")
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(Time,     (Time{14, 34, 15}), "<Time><value>14:34:15</value></Time>")
namespace {
typedef std::string string;
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(string, "hallo Du", "<string><value>hallo Du</value></string>")
}
namespace {
typedef std::string escapedString;
DEFINE_SIMPLE_DATATYPE_AS_CHILD_TEST(escapedString, "<b>\"STRING&amp; STRONG\"</b>", "<escapedString><value>&lt;b&gt;&quot;STRING&amp;amp; STRONG&quot;&lt;/b&gt;</value></escapedString>")
}


#define DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(type, datatypeValue, expectedResult) \
struct SerAsPlainChild_ ## type { \
  type value; \
  bool operator==(const SerAsPlainChild_ ## type& rhs) const { return value == rhs.value; } \
}; \
SERGUT_FUNCTION(SerAsPlainChild_ ## type, data, ar) \
{ \
  ar & sergut::plainChild & SERGUT_MMEMBER(data, value); \
} \
TEST_CASE("DeSerialize datatype " #type " as plain child in XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    SerAsPlainChild_ ## type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData(#type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const SerAsPlainChild_ ## type tpDeser1 = deser.deserializeData<SerAsPlainChild_ ## type>(#type); \
    CHECK(tpDeser1 == SerAsPlainChild_ ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
    sergut::XmlDeserializerTiny deser(expectedResult); \
    const SerAsPlainChild_ ## type tpDeser2 = deser.deserializeData<SerAsPlainChild_ ## type>(#type); \
    CHECK(tpDeser2 == SerAsPlainChild_ ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
    sergut::XmlDeserializerTiny2 deser(expectedResult); \
    const SerAsPlainChild_ ## type tpDeser2 = deser.deserializeData<SerAsPlainChild_ ## type>(#type); \
    CHECK(tpDeser2 == SerAsPlainChild_ ## type{ datatypeValue }); \
  } \
} \

DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(bool,       true, "<bool>true</bool>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(char,        'a', "<char>a</char>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(uint8_t,      19, "<uint8_t>19</uint8_t>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(int16_t,  -32767, "<int16_t>-32767</int16_t>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(uint16_t,  65535, "<uint16_t>65535</uint16_t>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(int32_t,  -32768, "<int32_t>-32768</int32_t>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(uint32_t,  65536, "<uint32_t>65536</uint32_t>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(int64_t,  -32768, "<int64_t>-32768</int64_t>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(uint64_t,  65536, "<uint64_t>65536</uint64_t>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(float,      2.25, "<float>2.25</float>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(double,     2.25, "<double>2.25</double>")
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(Time,     (Time{14, 34, 15}), "<Time>14:34:15</Time>")
namespace {
typedef std::string string;
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(string, "hallo Du", "<string>hallo Du</string>")
}
namespace {
typedef std::string escapedString;
DEFINE_SIMPLE_DATATYPE_AS_SINGLE_CHILD_TEST(escapedString, "<b>\"STRING&amp; STRONG\"</b>", "<escapedString>&lt;b&gt;&quot;STRING&amp;amp; STRONG&quot;&lt;/b&gt;</escapedString>")
}


#define DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(type, datatypeValue, expectedResult) \
TEST_CASE("DeSerialize basic datatype " #type " as attribute in XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeNestedData(#type, "value", sergut::XmlValueType::Attribute, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const type tpDeser1 = deser.deserializeNestedData<type, sergut::XmlValueType::Attribute>(#type, "value"); \
    CHECK(tpDeser1 == datatypeValue); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
    sergut::XmlDeserializerTiny deser(expectedResult); \
    const type tpDeser2 = deser.deserializeNestedData<type, sergut::XmlValueType::Attribute>(#type, "value"); \
    CHECK(tpDeser2 == datatypeValue); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
    sergut::XmlDeserializerTiny2 deser(expectedResult); \
    const type tpDeser2 = deser.deserializeNestedData<type, sergut::XmlValueType::Attribute>(#type, "value"); \
    CHECK(tpDeser2 == datatypeValue); \
  } \
} \

DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(bool,      false, "<bool value=\"false\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(char,        'a', "<char value=\"a\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(uint8_t,      19, "<uint8_t value=\"19\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(int16_t,  -32767, "<int16_t value=\"-32767\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(uint16_t,  65535, "<uint16_t value=\"65535\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(int32_t,  -32768, "<int32_t value=\"-32768\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(uint32_t,  65536, "<uint32_t value=\"65536\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(int64_t,  -32768, "<int64_t value=\"-32768\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(uint64_t,  65536, "<uint64_t value=\"65536\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(float,      2.25, "<float value=\"2.25\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(double,     2.25, "<double value=\"2.25\"/>")
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(Time,     (Time{14, 34, 15}), "<Time value=\"14:34:15\"/>")
namespace {
typedef std::string string;
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(string, "hallo Du", "<string value=\"hallo Du\"/>")
}
namespace {
typedef std::string escapedString;
DEFINE_BASIC_DATATYPE_AS_ATTRIBUTE_TEST(escapedString, "<b>\"STRING&amp; STRONG\"</b>", "<escapedString value=\"&lt;b&gt;&quot;STRING&amp;amp; STRONG&quot;&lt;/b&gt;\"/>")
}


#define DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(type, datatypeValue, expectedResult) \
TEST_CASE("DeSerialize basic datatype " #type " as child in XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeNestedData(#type, "value", sergut::XmlValueType::Child, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const type tpDeser1 = deser.deserializeNestedData<type, sergut::XmlValueType::Child>(#type, "value"); \
    CHECK(tpDeser1 == datatypeValue); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
    sergut::XmlDeserializerTiny deser(expectedResult); \
    const type tpDeser2 = deser.deserializeNestedData<type, sergut::XmlValueType::Child>(#type, "value"); \
    CHECK(tpDeser2 == datatypeValue); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
    sergut::XmlDeserializerTiny2 deser(expectedResult); \
    const type tpDeser2 = deser.deserializeNestedData<type, sergut::XmlValueType::Child>(#type, "value"); \
    CHECK(tpDeser2 == datatypeValue); \
  } \
} \

DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(bool,       true, "<bool><value>true</value></bool>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(char,        'a', "<char><value>a</value></char>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(uint8_t,      19, "<uint8_t><value>19</value></uint8_t>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(int16_t,  -32767, "<int16_t><value>-32767</value></int16_t>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(uint16_t,  65535, "<uint16_t><value>65535</value></uint16_t>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(int32_t,  -32768, "<int32_t><value>-32768</value></int32_t>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(uint32_t,  65536, "<uint32_t><value>65536</value></uint32_t>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(int64_t,  -32768, "<int64_t><value>-32768</value></int64_t>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(uint64_t,  65536, "<uint64_t><value>65536</value></uint64_t>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(float,      2.25, "<float><value>2.25</value></float>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(double,     2.25, "<double><value>2.25</value></double>")
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(Time,     (Time{14, 34, 15}), "<Time><value>14:34:15</value></Time>")
namespace {
typedef std::string string;
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(string, "hallo Du", "<string><value>hallo Du</value></string>")
}
namespace {
typedef std::string escapedString;
DEFINE_BASIC_DATATYPE_AS_CHILD_TEST(escapedString, "<b>\"STRING&amp; STRONG\"</b>", "<escapedString><value>&lt;b&gt;&quot;STRING&amp;amp; STRONG&quot;&lt;/b&gt;</value></escapedString>")
}


#define DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(type, datatypeValue, expectedResult) \
TEST_CASE("DeSerialize basic datatype " #type " as plain child in XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData(#type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const type tpDeser1 = deser.deserializeData<type>(#type); \
    CHECK(tpDeser1 == datatypeValue); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
    sergut::XmlDeserializerTiny deser(expectedResult); \
    const type tpDeser2 = deser.deserializeData<type>(#type); \
    CHECK(tpDeser2 == datatypeValue); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
    sergut::XmlDeserializerTiny2 deser(expectedResult); \
    const type tpDeser2 = deser.deserializeData<type>(#type); \
    CHECK(tpDeser2 == datatypeValue); \
  } \
} \

DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(bool,       true, "<bool>true</bool>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(uint8_t,      19, "<uint8_t>19</uint8_t>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(int16_t,  -32767, "<int16_t>-32767</int16_t>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(uint16_t,  65535, "<uint16_t>65535</uint16_t>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(int32_t,  -32768, "<int32_t>-32768</int32_t>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(uint32_t,  65536, "<uint32_t>65536</uint32_t>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(int64_t,  -32768, "<int64_t>-32768</int64_t>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(uint64_t,  65536, "<uint64_t>65536</uint64_t>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(float,      2.25, "<float>2.25</float>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(double,     2.25, "<double>2.25</double>")
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(Time,     (Time{14, 34, 15}), "<Time>14:34:15</Time>")
namespace {
typedef std::string string;
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(string, "hallo Du", "<string>hallo Du</string>")
}
namespace {
typedef std::string escapedString;
DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(escapedString, "<b>\"STRING&amp; STRONG\"</b>", "<escapedString>&lt;b&gt;&quot;STRING&amp;amp; STRONG&quot;&lt;/b&gt;</escapedString>")
}


#define DEFINE_COLLECTION_TEST_TYPE(type, collectionType, datatypeValue, expectedResult) \
struct Ser_ ## collectionType ## Of ## type { \
  std::collectionType<type> value; \
  bool operator==(const Ser_ ## collectionType ## Of ## type& rhs) const { return value == rhs.value; } \
}; \
SERGUT_FUNCTION(Ser_ ## collectionType ## Of ## type, data, ar) \
{ \
  ar & sergut::children & SERGUT_MMEMBER(data, value); \
} \

#define DEFINE_COLLECTION_TEST_CONTENT(type, collectionType, datatypeValue, expectedResult) \
  SECTION("Serialize " #collectionType) { \
    Ser_ ## collectionType ## Of ## type data{ std::collectionType<type>datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData("VectorOf" #type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize " #collectionType " with XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const Ser_ ## collectionType ## Of ## type tpDeser1 = deser.deserializeData<Ser_ ## collectionType ## Of ## type>("VectorOf" #type); \
    CHECK(tpDeser1 == Ser_ ## collectionType ## Of ## type{ std::collectionType<type>datatypeValue }); \
  } \
  SECTION("Deserialize " #collectionType " with XmlDeserializerTiny") { \
    sergut::XmlDeserializerTiny deser(expectedResult); \
    const Ser_ ## collectionType ## Of ## type tpDeser2 = deser.deserializeData<Ser_ ## collectionType ## Of ## type>("VectorOf" #type); \
    CHECK(tpDeser2 == Ser_ ## collectionType ## Of ## type{ std::collectionType<type>datatypeValue }); \
  } \
  SECTION("Deserialize " #collectionType " with XmlDeserializerTiny2") { \
    sergut::XmlDeserializerTiny2 deser(expectedResult); \
    const Ser_ ## collectionType ## Of ## type tpDeser2 = deser.deserializeData<Ser_ ## collectionType ## Of ## type>("VectorOf" #type); \
    CHECK(tpDeser2 == Ser_ ## collectionType ## Of ## type{ std::collectionType<type>datatypeValue }); \
  } \

#define DEFINE_COLLECTION_TEST(type, datatypeValue, expectedResult) \
  DEFINE_COLLECTION_TEST_TYPE(type, list, datatypeValue, expectedResult) \
  DEFINE_COLLECTION_TEST_TYPE(type, set, datatypeValue, expectedResult) \
  DEFINE_COLLECTION_TEST_TYPE(type, vector, datatypeValue, expectedResult) \
  TEST_CASE("DeSerialize collection of " #type " as XML", "[sergut]") \
  { \
    DEFINE_COLLECTION_TEST_CONTENT(type, list, datatypeValue, expectedResult) \
    DEFINE_COLLECTION_TEST_CONTENT(type, set, datatypeValue, expectedResult) \
    DEFINE_COLLECTION_TEST_CONTENT(type, vector, datatypeValue, expectedResult) \
  } \

DEFINE_COLLECTION_TEST(bool,     ({false, true}), "<VectorOfbool><value>false</value><value>true</value></VectorOfbool>")
DEFINE_COLLECTION_TEST(char,     ({'a', 'b', 'c'}), "<VectorOfchar><value>a</value><value>b</value><value>c</value></VectorOfchar>")
DEFINE_COLLECTION_TEST(uint8_t,  ({0, 17, 255}), "<VectorOfuint8_t><value>0</value><value>17</value><value>255</value></VectorOfuint8_t>")
DEFINE_COLLECTION_TEST(int16_t,  ({-32768, 17, 32767}), "<VectorOfint16_t><value>-32768</value><value>17</value><value>32767</value></VectorOfint16_t>")
DEFINE_COLLECTION_TEST(uint16_t, ({0, 17, 65535}), "<VectorOfuint16_t><value>0</value><value>17</value><value>65535</value></VectorOfuint16_t>")
DEFINE_COLLECTION_TEST(int32_t,  ({-2147483648, 17, 2147483647}), "<VectorOfint32_t><value>-2147483648</value><value>17</value><value>2147483647</value></VectorOfint32_t>")
DEFINE_COLLECTION_TEST(uint32_t, ({0, 17, 4294967295}), "<VectorOfuint32_t><value>0</value><value>17</value><value>4294967295</value></VectorOfuint32_t>")
DEFINE_COLLECTION_TEST(int64_t,  ({-9223372036854775807LL, 17, 9223372036854775807LL}), "<VectorOfint64_t><value>-9223372036854775807</value><value>17</value><value>9223372036854775807</value></VectorOfint64_t>")
DEFINE_COLLECTION_TEST(uint64_t, ({0, 17, 9223372036854775808ULL}), "<VectorOfuint64_t><value>0</value><value>17</value><value>9223372036854775808</value></VectorOfuint64_t>")
DEFINE_COLLECTION_TEST(float,    ({-17.25, 0.0, 0.0000025}), "<VectorOffloat><value>-17.25</value><value>0</value><value>2.5e-06</value></VectorOffloat>")
DEFINE_COLLECTION_TEST(double,   ({-17.25, 0.0, 0.0000025}), "<VectorOfdouble><value>-17.25</value><value>0</value><value>2.5e-06</value></VectorOfdouble>")
DEFINE_COLLECTION_TEST(Time,     ({Time{0,0,0}, Time{14,34,15}, Time{15,55,55}}), "<VectorOfTime><value>0:00:00</value><value>14:34:15</value><value>15:55:55</value></VectorOfTime>")
namespace {
typedef std::string string;
DEFINE_COLLECTION_TEST(string, ({"Hallo", "Liebe", "Welt"}), "<VectorOfstring><value>Hallo</value><value>Liebe</value><value>Welt</value></VectorOfstring>")
}
namespace {
typedef std::string escapedString;
DEFINE_COLLECTION_TEST(escapedString, ({"<b>\"STRING&amp;\"</b>", "<b>\"STRING&uuml;\"</b>"}),
                   "<VectorOfescapedString><value>&lt;b&gt;&quot;STRING&amp;amp;&quot;&lt;/b&gt;</value><value>&lt;b&gt;&quot;STRING&amp;uuml;&quot;&lt;/b&gt;</value></VectorOfescapedString>")
}


#define DEFINE_COLLECTION_SIBLINGS_TEST_TYPE(type, collectionType, expectedResult) \
struct Ser_ ## collectionType ## SiblingsOf ## type { \
  std::collectionType<type> valueA; \
  std::collectionType<type> valueB; \
  bool operator==(const Ser_ ## collectionType ## SiblingsOf ## type& rhs) const { return valueA == rhs.valueA && valueB == rhs.valueB; } \
}; \
SERGUT_FUNCTION(Ser_ ## collectionType ## SiblingsOf ## type, data, ar) \
{ \
  ar & sergut::children & SERGUT_MMEMBER(data, valueA) & SERGUT_MMEMBER(data, valueB); \
} \

#define DEFINE_COLLECTION_SIBLINGS_TEST_CONTENT(type, collectionType, datatypeValueA, datatypeValueB, expectedResult) \
  SECTION("Serialize sibling " #collectionType) { \
    Ser_ ## collectionType ## SiblingsOf ## type data{ std::collectionType<type>datatypeValueA, std::collectionType<type>datatypeValueB }; \
    sergut::XmlSerializer ser; \
    ser.serializeData("VectorsOf" #type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize sibling " #collectionType " with XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const Ser_ ## collectionType ## SiblingsOf ## type tpDeser1 = deser.deserializeData<Ser_ ## collectionType ## SiblingsOf ## type>("VectorsOf" #type); \
    CHECK(tpDeser1 == (Ser_ ## collectionType ## SiblingsOf ## type{ std::collectionType<type>datatypeValueA, std::collectionType<type>datatypeValueB })); \
  } \
  SECTION("Deserialize sibling " #collectionType " with XmlDeserializerTiny") { \
    sergut::XmlDeserializerTiny deser(expectedResult); \
    const Ser_ ## collectionType ## SiblingsOf ## type tpDeser2 = deser.deserializeData<Ser_ ## collectionType ## SiblingsOf ## type>("VectorsOf" #type); \
    CHECK(tpDeser2 == (Ser_ ## collectionType ## SiblingsOf ## type{ std::collectionType<type>datatypeValueA, std::collectionType<type>datatypeValueB })); \
  } \
  SECTION("Deserialize sibling " #collectionType " with XmlDeserializerTiny2") { \
    sergut::XmlDeserializerTiny2 deser(expectedResult); \
    const Ser_ ## collectionType ## SiblingsOf ## type tpDeser2 = deser.deserializeData<Ser_ ## collectionType ## SiblingsOf ## type>("VectorsOf" #type); \
    CHECK(tpDeser2 == (Ser_ ## collectionType ## SiblingsOf ## type{ std::collectionType<type>datatypeValueA, std::collectionType<type>datatypeValueB })); \
  } \

#define DEFINE_COLLECTION_SIBLINGS_TEST(type, datatypeValueA, datatypeValueB, expectedResult) \
  DEFINE_COLLECTION_SIBLINGS_TEST_TYPE(type, list, expectedResult) \
  DEFINE_COLLECTION_SIBLINGS_TEST_TYPE(type, set, expectedResult) \
  DEFINE_COLLECTION_SIBLINGS_TEST_TYPE(type, vector, expectedResult) \
  TEST_CASE("DeSerialize sibling collections of " #type " as XML", "[sergut]") \
  { \
    DEFINE_COLLECTION_SIBLINGS_TEST_CONTENT(type, list, datatypeValueA, datatypeValueB, expectedResult) \
    DEFINE_COLLECTION_SIBLINGS_TEST_CONTENT(type, set, datatypeValueA, datatypeValueB, expectedResult) \
    DEFINE_COLLECTION_SIBLINGS_TEST_CONTENT(type, vector, datatypeValueA, datatypeValueB, expectedResult) \
  } \

DEFINE_COLLECTION_SIBLINGS_TEST(bool,     ({false, true}), ({false, true}), "<VectorsOfbool><valueA>false</valueA><valueA>true</valueA><valueB>false</valueB><valueB>true</valueB></VectorsOfbool>")
DEFINE_COLLECTION_SIBLINGS_TEST(char,     ({'a', 'b'}), ({'a', 'c'}), "<VectorsOfchar><valueA>a</valueA><valueA>b</valueA><valueB>a</valueB><valueB>c</valueB></VectorsOfchar>")
DEFINE_COLLECTION_SIBLINGS_TEST(uint8_t,  ({0, 17}), ({1, 255}), "<VectorsOfuint8_t><valueA>0</valueA><valueA>17</valueA><valueB>1</valueB><valueB>255</valueB></VectorsOfuint8_t>")
DEFINE_COLLECTION_SIBLINGS_TEST(int16_t,  ({-32768, 17}),({-2000, 32767}), "<VectorsOfint16_t><valueA>-32768</valueA><valueA>17</valueA><valueB>-2000</valueB><valueB>32767</valueB></VectorsOfint16_t>")
DEFINE_COLLECTION_SIBLINGS_TEST(uint16_t, ({0, 17}),({20, 65535}), "<VectorsOfuint16_t><valueA>0</valueA><valueA>17</valueA><valueB>20</valueB><valueB>65535</valueB></VectorsOfuint16_t>")
DEFINE_COLLECTION_SIBLINGS_TEST(int32_t,  ({-2147483648, 17}), ({-2147483648, 2147483647}), "<VectorsOfint32_t><valueA>-2147483648</valueA><valueA>17</valueA><valueB>-2147483648</valueB><valueB>2147483647</valueB></VectorsOfint32_t>")
DEFINE_COLLECTION_SIBLINGS_TEST(uint32_t, ({0, 17}),({0, 4294967295}), "<VectorsOfuint32_t><valueA>0</valueA><valueA>17</valueA><valueB>0</valueB><valueB>4294967295</valueB></VectorsOfuint32_t>")
DEFINE_COLLECTION_SIBLINGS_TEST(int64_t,  ({-9223372036854775807LL, 17}),({17, 9223372036854775807LL}), "<VectorsOfint64_t><valueA>-9223372036854775807</valueA><valueA>17</valueA><valueB>17</valueB><valueB>9223372036854775807</valueB></VectorsOfint64_t>")
DEFINE_COLLECTION_SIBLINGS_TEST(uint64_t, ({0, 17}), ({17, 9223372036854775808ULL}), "<VectorsOfuint64_t><valueA>0</valueA><valueA>17</valueA><valueB>17</valueB><valueB>9223372036854775808</valueB></VectorsOfuint64_t>")
DEFINE_COLLECTION_SIBLINGS_TEST(float,    ({-17.25, 0.0}), ({-18.125, 0.0000025}), "<VectorsOffloat><valueA>-17.25</valueA><valueA>0</valueA><valueB>-18.125</valueB><valueB>2.5e-06</valueB></VectorsOffloat>")
DEFINE_COLLECTION_SIBLINGS_TEST(double,   ({-17.25, 0.0}), ({0.0, 0.0000025}), "<VectorsOfdouble><valueA>-17.25</valueA><valueA>0</valueA><valueB>0</valueB><valueB>2.5e-06</valueB></VectorsOfdouble>")
DEFINE_COLLECTION_SIBLINGS_TEST(Time,     ({Time{0,0,0}, Time{14,34,15}}), ({Time{15,55,54}, Time{15,55,55}}), "<VectorsOfTime><valueA>0:00:00</valueA><valueA>14:34:15</valueA><valueB>15:55:54</valueB><valueB>15:55:55</valueB></VectorsOfTime>")
namespace {
typedef std::string string;
DEFINE_COLLECTION_SIBLINGS_TEST(string, ({"Hallo", "Liebe"}), ({"Hallo", "Welt"}), "<VectorsOfstring><valueA>Hallo</valueA><valueA>Liebe</valueA><valueB>Hallo</valueB><valueB>Welt</valueB></VectorsOfstring>")
}
namespace {
typedef std::string escapedString;
DEFINE_COLLECTION_SIBLINGS_TEST(escapedString, ({"<b>\"STRING&amp;\"</b>"}), ({"<b>\"STRING&uuml;\"</b>"}),
                   "<VectorsOfescapedString><valueA>&lt;b&gt;&quot;STRING&amp;amp;&quot;&lt;/b&gt;</valueA><valueB>&lt;b&gt;&quot;STRING&amp;uuml;&quot;&lt;/b&gt;</valueB></VectorsOfescapedString>")
}


#define DEFINE_EMPTY_VECTOR_TEST(type) \
struct Ser_EmptyVectorOf ## type { \
  std::vector<type> value; \
  bool operator==(const Ser_EmptyVectorOf ## type& rhs) const { return value == rhs.value; } \
}; \
SERGUT_FUNCTION(Ser_EmptyVectorOf ## type, data, ar) \
{ \
  ar & sergut::children & SERGUT_MMEMBER(data, value); \
} \
TEST_CASE("DeSerialize empty vector of " #type " as XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    Ser_EmptyVectorOf ## type data; \
    sergut::XmlSerializer ser; \
    ser.serializeData("VectorOf" #type, data); \
    CHECK(ser.str() == "<VectorOf" #type "></VectorOf" #type ">"); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser("<VectorOf" #type "></VectorOf" #type ">"); \
    const Ser_EmptyVectorOf ## type tpDeser1 = deser.deserializeData<Ser_EmptyVectorOf ## type>("VectorOf" #type); \
    CHECK(tpDeser1 == Ser_EmptyVectorOf ## type()); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
  sergut::XmlDeserializerTiny deser("<VectorOf" #type "></VectorOf" #type ">"); \
  const Ser_EmptyVectorOf ## type tpDeser2 = deser.deserializeData<Ser_EmptyVectorOf ## type>("VectorOf" #type); \
  CHECK(tpDeser2 == Ser_EmptyVectorOf ## type()); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
  sergut::XmlDeserializerTiny2 deser("<VectorOf" #type "></VectorOf" #type ">"); \
  const Ser_EmptyVectorOf ## type tpDeser2 = deser.deserializeData<Ser_EmptyVectorOf ## type>("VectorOf" #type); \
  CHECK(tpDeser2 == Ser_EmptyVectorOf ## type()); \
  } \
} \

DEFINE_EMPTY_VECTOR_TEST(bool)
DEFINE_EMPTY_VECTOR_TEST(char)
DEFINE_EMPTY_VECTOR_TEST(uint8_t)
DEFINE_EMPTY_VECTOR_TEST(int16_t)
DEFINE_EMPTY_VECTOR_TEST(uint16_t)
DEFINE_EMPTY_VECTOR_TEST(int32_t)
DEFINE_EMPTY_VECTOR_TEST(uint32_t)
DEFINE_EMPTY_VECTOR_TEST(int64_t)
DEFINE_EMPTY_VECTOR_TEST(uint64_t)
DEFINE_EMPTY_VECTOR_TEST(float)
DEFINE_EMPTY_VECTOR_TEST(double)
DEFINE_EMPTY_VECTOR_TEST(Time)


#define DEFINE_NESTED_VECTOR_TEST(type, datatypeValue, expectedResult) \
struct Ser_NestedVectorOf ## type { \
  std::vector<type> values; \
  bool operator==(const Ser_NestedVectorOf ## type& rhs) const { return values == rhs.values; } \
}; \
SERGUT_FUNCTION(Ser_NestedVectorOf ## type, data, ar) \
{ \
  ar & sergut::children & SERGUT_NESTED_MMEMBER(data, values, value); \
} \
TEST_CASE("DeSerialize nested vector of " #type " as XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    Ser_NestedVectorOf ## type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData("NestedVectorOf" #type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const Ser_NestedVectorOf ## type tpDeser1 = deser.deserializeData<Ser_NestedVectorOf ## type>("NestedVectorOf" #type); \
    CHECK(tpDeser1 == Ser_NestedVectorOf ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
  sergut::XmlDeserializerTiny deser(expectedResult); \
  const Ser_NestedVectorOf ## type tpDeser2 = deser.deserializeData<Ser_NestedVectorOf ## type>("NestedVectorOf" #type); \
  CHECK(tpDeser2 == Ser_NestedVectorOf ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
  sergut::XmlDeserializerTiny2 deser(expectedResult); \
  const Ser_NestedVectorOf ## type tpDeser2 = deser.deserializeData<Ser_NestedVectorOf ## type>("NestedVectorOf" #type); \
  CHECK(tpDeser2 == Ser_NestedVectorOf ## type{ datatypeValue }); \
  } \
} \

DEFINE_NESTED_VECTOR_TEST(bool,     (std::vector<bool>{false, true}), "<NestedVectorOfbool><values><value>false</value><value>true</value></values></NestedVectorOfbool>")
DEFINE_NESTED_VECTOR_TEST(char,     (std::vector<char>{'a', 'b', 'c'}), "<NestedVectorOfchar><values><value>a</value><value>b</value><value>c</value></values></NestedVectorOfchar>")
DEFINE_NESTED_VECTOR_TEST(uint8_t,  (std::vector<uint8_t>{0, 17, 255}), "<NestedVectorOfuint8_t><values><value>0</value><value>17</value><value>255</value></values></NestedVectorOfuint8_t>")
DEFINE_NESTED_VECTOR_TEST(int16_t,  (std::vector<int16_t>{-32768, 17, 32767}), "<NestedVectorOfint16_t><values><value>-32768</value><value>17</value><value>32767</value></values></NestedVectorOfint16_t>")
DEFINE_NESTED_VECTOR_TEST(uint16_t, (std::vector<uint16_t>{0, 17, 65535}), "<NestedVectorOfuint16_t><values><value>0</value><value>17</value><value>65535</value></values></NestedVectorOfuint16_t>")
DEFINE_NESTED_VECTOR_TEST(int32_t,  (std::vector<int32_t>{-2147483648, 17, 2147483647}), "<NestedVectorOfint32_t><values><value>-2147483648</value><value>17</value><value>2147483647</value></values></NestedVectorOfint32_t>")
DEFINE_NESTED_VECTOR_TEST(uint32_t, (std::vector<uint32_t>{0, 17, 4294967295}), "<NestedVectorOfuint32_t><values><value>0</value><value>17</value><value>4294967295</value></values></NestedVectorOfuint32_t>")
DEFINE_NESTED_VECTOR_TEST(int64_t,  (std::vector<int64_t>{-9223372036854775807LL, 17, 9223372036854775807LL}), "<NestedVectorOfint64_t><values><value>-9223372036854775807</value><value>17</value><value>9223372036854775807</value></values></NestedVectorOfint64_t>")
DEFINE_NESTED_VECTOR_TEST(uint64_t, (std::vector<uint64_t>{0, 17, 9223372036854775808ULL}), "<NestedVectorOfuint64_t><values><value>0</value><value>17</value><value>9223372036854775808</value></values></NestedVectorOfuint64_t>")
DEFINE_NESTED_VECTOR_TEST(float,    (std::vector<float>{0.0, -17.25, 0.0000025}), "<NestedVectorOffloat><values><value>0</value><value>-17.25</value><value>2.5e-06</value></values></NestedVectorOffloat>")
DEFINE_NESTED_VECTOR_TEST(double,   (std::vector<double>{0.0, -17.25, 0.0000025}), "<NestedVectorOfdouble><values><value>0</value><value>-17.25</value><value>2.5e-06</value></values></NestedVectorOfdouble>")
DEFINE_NESTED_VECTOR_TEST(Time,     (std::vector<Time>{Time{15,45,45},Time{0,0,0},Time{23,59,59}}), "<NestedVectorOfTime><values><value>15:45:45</value><value>0:00:00</value><value>23:59:59</value></values></NestedVectorOfTime>")
namespace {
typedef std::string string;
DEFINE_NESTED_VECTOR_TEST(string, (std::vector<string>{"hallo", "liebe", "Welt"}), "<NestedVectorOfstring><values><value>hallo</value><value>liebe</value><value>Welt</value></values></NestedVectorOfstring>")
}
namespace {
typedef std::string escapedString;
DEFINE_NESTED_VECTOR_TEST(escapedString, (std::vector<escapedString>{"<b>\"STRING&amp;\"</b>", "<b>\"STRING&uuml;\"</b>"}),
                   "<NestedVectorOfescapedString><values><value>&lt;b&gt;&quot;STRING&amp;amp;&quot;&lt;/b&gt;</value><value>&lt;b&gt;&quot;STRING&amp;uuml;&quot;&lt;/b&gt;</value></values></NestedVectorOfescapedString>")
}


#define DEFINE_NESTED_SIMPLE_TEST(type, datatypeValue, expectedResult) \
struct Ser_Nested ## type { \
  type value; \
  bool operator==(const Ser_Nested ## type& rhs) const { return value == rhs.value; } \
}; \
SERGUT_FUNCTION(Ser_Nested ## type, data, ar) \
{ \
  ar & sergut::children & SERGUT_NESTED_MMEMBER(data, value, nested); \
} \
TEST_CASE("DeSerialize nested " #type " as XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    Ser_Nested ## type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData("Nested" #type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const Ser_Nested ## type tpDeser1 = deser.deserializeData<Ser_Nested ## type>("Nested" #type); \
    CHECK(tpDeser1 == Ser_Nested ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
  sergut::XmlDeserializerTiny deser(expectedResult); \
  const Ser_Nested ## type tpDeser2 = deser.deserializeData<Ser_Nested ## type>("Nested" #type); \
  CHECK(tpDeser2 == Ser_Nested ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
  sergut::XmlDeserializerTiny2 deser(expectedResult); \
  const Ser_Nested ## type tpDeser2 = deser.deserializeData<Ser_Nested ## type>("Nested" #type); \
  CHECK(tpDeser2 == Ser_Nested ## type{ datatypeValue }); \
  } \
} \

DEFINE_NESTED_SIMPLE_TEST(bool,     true, "<Nestedbool><value><nested>true</nested></value></Nestedbool>")
DEFINE_NESTED_SIMPLE_TEST(char,      'a', "<Nestedchar><value><nested>a</nested></value></Nestedchar>")
DEFINE_NESTED_SIMPLE_TEST(uint8_t,  17, "<Nesteduint8_t><value><nested>17</nested></value></Nesteduint8_t>")
DEFINE_NESTED_SIMPLE_TEST(int16_t,  -32768, "<Nestedint16_t><value><nested>-32768</nested></value></Nestedint16_t>")
DEFINE_NESTED_SIMPLE_TEST(uint16_t, 65535, "<Nesteduint16_t><value><nested>65535</nested></value></Nesteduint16_t>")
DEFINE_NESTED_SIMPLE_TEST(int32_t,  -2147483648, "<Nestedint32_t><value><nested>-2147483648</nested></value></Nestedint32_t>")
DEFINE_NESTED_SIMPLE_TEST(uint32_t, 4294967295, "<Nesteduint32_t><value><nested>4294967295</nested></value></Nesteduint32_t>")
DEFINE_NESTED_SIMPLE_TEST(int64_t,  -9223372036854775807LL, "<Nestedint64_t><value><nested>-9223372036854775807</nested></value></Nestedint64_t>")
DEFINE_NESTED_SIMPLE_TEST(uint64_t, 9223372036854775808ULL, "<Nesteduint64_t><value><nested>9223372036854775808</nested></value></Nesteduint64_t>")
DEFINE_NESTED_SIMPLE_TEST(float,    -17.25, "<Nestedfloat><value><nested>-17.25</nested></value></Nestedfloat>")
DEFINE_NESTED_SIMPLE_TEST(double,   -17.25, "<Nesteddouble><value><nested>-17.25</nested></value></Nesteddouble>")
DEFINE_NESTED_SIMPLE_TEST(Time,   (Time{1,45,37}), "<NestedTime><value><nested>1:45:37</nested></value></NestedTime>")
namespace {
  typedef std::string string;
  DEFINE_NESTED_SIMPLE_TEST(string, "liebe", "<Nestedstring><value><nested>liebe</nested></value></Nestedstring>")
}
namespace {
typedef std::string escapedString;
DEFINE_NESTED_SIMPLE_TEST(escapedString, "<b>\"STRING&amp;\"</b>",
                          "<NestedescapedString><value><nested>&lt;b&gt;&quot;STRING&amp;amp;&quot;&lt;/b&gt;</nested></value></NestedescapedString>")
}


#define DEFINE_DOUBLE_NESTED_TEST(type, datatypeValue, expectedResult) \
struct Ser_DoubleNested ## type { \
  type value; \
  bool operator==(const Ser_DoubleNested ## type& rhs) const { return value == rhs.value; } \
}; \
SERGUT_FUNCTION(Ser_DoubleNested ## type, data, ar) \
{ \
  ar & sergut::children & Archive::toNamedMember("outerNested", Archive::toNestedMember("innerNested", Archive::toNestedMember("value", data.value, true), true), true); \
} \
TEST_CASE("DeSerialize nested member of " #type " as XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    Ser_DoubleNested ## type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData("DoubleNested" #type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const Ser_DoubleNested ## type tpDeser1 = deser.deserializeData<Ser_DoubleNested ## type>("DoubleNested" #type); \
    CHECK(tpDeser1 == Ser_DoubleNested ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
  sergut::XmlDeserializerTiny deser(expectedResult); \
  const Ser_DoubleNested ## type tpDeser2 = deser.deserializeData<Ser_DoubleNested ## type>("DoubleNested" #type); \
  CHECK(tpDeser2 == Ser_DoubleNested ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
  sergut::XmlDeserializerTiny2 deser(expectedResult); \
  const Ser_DoubleNested ## type tpDeser2 = deser.deserializeData<Ser_DoubleNested ## type>("DoubleNested" #type); \
  CHECK(tpDeser2 == Ser_DoubleNested ## type{ datatypeValue }); \
  } \
} \

DEFINE_DOUBLE_NESTED_TEST(bool,     false, "<DoubleNestedbool><outerNested><innerNested><value>false</value></innerNested></outerNested></DoubleNestedbool>")
DEFINE_DOUBLE_NESTED_TEST(char,      'a', "<DoubleNestedchar><outerNested><innerNested><value>a</value></innerNested></outerNested></DoubleNestedchar>")
DEFINE_DOUBLE_NESTED_TEST(uint8_t,  17, "<DoubleNesteduint8_t><outerNested><innerNested><value>17</value></innerNested></outerNested></DoubleNesteduint8_t>")
DEFINE_DOUBLE_NESTED_TEST(int16_t,  -32768, "<DoubleNestedint16_t><outerNested><innerNested><value>-32768</value></innerNested></outerNested></DoubleNestedint16_t>")
DEFINE_DOUBLE_NESTED_TEST(uint16_t, 65535, "<DoubleNesteduint16_t><outerNested><innerNested><value>65535</value></innerNested></outerNested></DoubleNesteduint16_t>")
DEFINE_DOUBLE_NESTED_TEST(int32_t,  -2147483648, "<DoubleNestedint32_t><outerNested><innerNested><value>-2147483648</value></innerNested></outerNested></DoubleNestedint32_t>")
DEFINE_DOUBLE_NESTED_TEST(uint32_t, 4294967295, "<DoubleNesteduint32_t><outerNested><innerNested><value>4294967295</value></innerNested></outerNested></DoubleNesteduint32_t>")
DEFINE_DOUBLE_NESTED_TEST(int64_t,  -9223372036854775807LL, "<DoubleNestedint64_t><outerNested><innerNested><value>-9223372036854775807</value></innerNested></outerNested></DoubleNestedint64_t>")
DEFINE_DOUBLE_NESTED_TEST(uint64_t, 9223372036854775808ULL, "<DoubleNesteduint64_t><outerNested><innerNested><value>9223372036854775808</value></innerNested></outerNested></DoubleNesteduint64_t>")
DEFINE_DOUBLE_NESTED_TEST(float,    -17.25, "<DoubleNestedfloat><outerNested><innerNested><value>-17.25</value></innerNested></outerNested></DoubleNestedfloat>")
DEFINE_DOUBLE_NESTED_TEST(double,   -17.25, "<DoubleNesteddouble><outerNested><innerNested><value>-17.25</value></innerNested></outerNested></DoubleNesteddouble>")
DEFINE_DOUBLE_NESTED_TEST(Time,     (Time{5,33,55}), "<DoubleNestedTime><outerNested><innerNested><value>5:33:55</value></innerNested></outerNested></DoubleNestedTime>")
namespace {
  typedef std::string string;
  DEFINE_DOUBLE_NESTED_TEST(string, "liebe", "<DoubleNestedstring><outerNested><innerNested><value>liebe</value></innerNested></outerNested></DoubleNestedstring>")
}
namespace {
typedef std::string escapedString;
DEFINE_DOUBLE_NESTED_TEST(escapedString, "<b>\"STRING&amp;\"</b>",
                          "<DoubleNestedescapedString><outerNested><innerNested><value>&lt;b&gt;&quot;STRING&amp;amp;&quot;&lt;/b&gt;</value></innerNested></outerNested></DoubleNestedescapedString>")
}


TEST_CASE("Deserialize XML snippets of simple type", "[sergut]")
{
  constexpr int REPETITION_COUNT=10;
  GIVEN("XML with a list") {
    std::string xml = "<root>";
    for(int i = 0; i < REPETITION_COUNT; ++i) {
      xml += "<data>" + std::to_string(i) + "</data>";
    }
    xml += "</root>";
    WHEN("Starting to read out the XML snippets") {
      std::unique_ptr<sergut::xml::PullParser> pullParserHolder = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
      sergut::xml::PullParser& pullParser = *pullParserHolder;
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("root"));
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("data"));
      THEN("The elements are correctly deserialized") {
        for(int i = 0; i < REPETITION_COUNT; ++i) {
          CHECK(sergut::XmlDeserializer::deserializeFromSnippet<int>("data", pullParser) == i);
        }
        CHECK(pullParser.getCurrentTokenType() == sergut::xml::ParseTokenType::CloseTag);
        CHECK(pullParser.getCurrentTagName() == std::string("root"));
      }
    }
  }
}


TEST_CASE("Deserialize XML snippets of simple nested type as attribute", "[sergut]")
{
  constexpr int REPETITION_COUNT=10;
  GIVEN("XML with a list") {
    std::string xml = "<root>";
    for(int i = 0; i < REPETITION_COUNT; ++i) {
      xml += "<data value=\"" + std::to_string(i) + "\"/>";
    }
    xml += "</root>";
    WHEN("Starting to read out the XML snippets") {
      std::unique_ptr<sergut::xml::PullParser> pullParserHolder = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
      sergut::xml::PullParser& pullParser = *pullParserHolder;
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("root"));
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("data"));
      THEN("The elements are correctly deserialized") {
        for(int i = 0; i < REPETITION_COUNT; ++i) {
          CHECK((sergut::XmlDeserializer::deserializeNestedFromSnippet<int, sergut::XmlValueType::Attribute>(
                   "data", "value", pullParser)) == i);
        }
        CHECK(pullParser.getCurrentTokenType() == sergut::xml::ParseTokenType::CloseTag);
        CHECK(pullParser.getCurrentTagName() == std::string("root"));
      }
    }
  }
}


TEST_CASE("Deserialize XML snippets of simple nested type as children", "[sergut]")
{
  constexpr int REPETITION_COUNT=10;
  GIVEN("XML with a list") {
    std::string xml = "<root>";
    for(int i = 0; i < REPETITION_COUNT; ++i) {
      xml += "<data><value>" + std::to_string(i) + "</value></data>";
    }
    xml += "</root>";
    WHEN("Starting to read out the XML snippets") {
      std::unique_ptr<sergut::xml::PullParser> pullParserHolder = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
      sergut::xml::PullParser& pullParser = *pullParserHolder;
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("root"));
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("data"));
      THEN("The elements are correctly deserialized with the explicit function") {
        for(int i = 0; i < REPETITION_COUNT; ++i) {
          CHECK((sergut::XmlDeserializer::deserializeNestedFromSnippet<int, sergut::XmlValueType::Child>(
                  "data", "value", pullParser)) == i);
        }
        CHECK(pullParser.getCurrentTokenType() == sergut::xml::ParseTokenType::CloseTag);
        CHECK(pullParser.getCurrentTagName() == std::string("root"));
      }
      THEN("The elements are correctly deserialized with the implicit function") {
        for(int i = 0; i < REPETITION_COUNT; ++i) {
          CHECK(sergut::XmlDeserializer::deserializeNestedFromSnippet<int>("data",
                                                                           "value",
                                                                           pullParser) == i);
        }
        CHECK(pullParser.getCurrentTokenType() == sergut::xml::ParseTokenType::CloseTag);
        CHECK(pullParser.getCurrentTagName() == std::string("root"));
      }
    }
  }
}


struct XmlSnippetData {
  int v1;
  int v2;
  bool operator==(const XmlSnippetData& rhs) const { return v1 == rhs.v1 && v2 == rhs.v2; }
};
SERGUT_FUNCTION(XmlSnippetData, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, v1)
      & SERGUT_MMEMBER(data, v2);
}

TEST_CASE("Deserialize XML snippets of datastructure as attributes", "[sergut]")
{
  constexpr int REPETITION_COUNT=10;
  GIVEN("XML with a list") {
    std::string xml = "<root>";
    for(int i = 0; i < REPETITION_COUNT; ++i) {
      xml += "<data v1=\"" + std::to_string(i) + "\" v2=\"" + std::to_string(i+1) + "\"></data>";
    }
    xml += "</root>";
    WHEN("Starting to read out the XML snippets") {
      std::unique_ptr<sergut::xml::PullParser> pullParserHolder = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
      sergut::xml::PullParser& pullParser = *pullParserHolder;
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("root"));
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("data"));
      THEN("The elements are correctly deserialized with the explicit function") {
        for(int i = 0; i < REPETITION_COUNT; ++i) {
          CHECK(sergut::XmlDeserializer::deserializeFromSnippet<XmlSnippetData>("data",
                                                                                pullParser)
                == (XmlSnippetData{i, i+1}));
        }
        CHECK(pullParser.getCurrentTokenType() == sergut::xml::ParseTokenType::CloseTag);
        CHECK(pullParser.getCurrentTagName() == std::string("root"));
      }
    }
  }
}


TEST_CASE("Deserialize XML snippets of nested datastructure as attributes", "[sergut]")
{
  constexpr int REPETITION_COUNT=10;
  GIVEN("XML with a list") {
    std::string xml = "<root>";
    for(int i = 0; i < REPETITION_COUNT; ++i) {
      xml += "<data><value v1=\"" + std::to_string(i) + "\" v2=\"" + std::to_string(i+1) + "\"/></data>";
    }
    xml += "</root>";
    WHEN("Starting to read out the XML snippets") {
      std::unique_ptr<sergut::xml::PullParser> pullParserHolder = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
      sergut::xml::PullParser& pullParser = *pullParserHolder;
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("root"));
      CHECK(pullParser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
      CHECK(pullParser.getCurrentTagName() == std::string("data"));
      THEN("The elements are correctly deserialized with the explicit function") {
        for(int i = 0; i < REPETITION_COUNT; ++i) {
          CHECK(sergut::XmlDeserializer::deserializeNestedFromSnippet<XmlSnippetData>("data", "value", pullParser)
                == (XmlSnippetData{i, i+1}));
        }
        CHECK(pullParser.getCurrentTokenType() == sergut::xml::ParseTokenType::CloseTag);
        CHECK(pullParser.getCurrentTagName() == std::string("root"));
      }
    }
  }
}


struct SavepointTest {
  SavepointTest() = default;
  SavepointTest(int aAtt, int aV) : att(aAtt), v(aV) { }
  bool operator==(const SavepointTest& rhs) const { return att == rhs.att && v == rhs.v; }
  int att = 0;
  int v = 0;
};
SERGUT_FUNCTION(SavepointTest, data, ar) {
  ar
      & SERGUT_MMEMBER(data, att)
      & sergut::children
      & SERGUT_MMEMBER(data, v);
}

TEST_CASE("Deserialize incomplete XML with resume", "[sergut]")
{
  const std::vector<std::tuple<std::string, std::string, int>> testData{
    std::make_tuple( std::string("<root><inner a"), std::string("tt=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner></root>"), 0 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><"), std::string("v>1</v></inner><inner att=\"1\"><v>1</v></inner></root>"), 0 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v>"), std::string("</inner><inner att=\"1\"><v>1</v></inner></root>"), 0 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner"), std::string("><inner att=\"1\"><v>1</v></inner></root>"), 0 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner>"), std::string("<inner att=\"1\"><v>1</v></inner></root>"), 0 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner"), std::string(" att=\"1\"><v>1</v></inner></root>"), 0 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner a"), std::string("tt=\"1\"><v>1</v></inner></root>"), 1 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\">"), std::string("<v>1</v></inner></root>"), 1 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1"), std::string("</v></inner></root>"), 1 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v>"), std::string("</inner></root>"), 1 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner"), std::string("></root>"), 1 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner>"), std::string("</root>"), 1 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner></"), std::string("root>"), 1 ),
    std::make_tuple( std::string("<root><inner att=\"1\"><v>1</v></inner><inner att=\"1\"><v>1</v></inner></root"), std::string(">"), 1 )
  };
  for(const auto& values: testData) {
    const std::string& firstPart = std::get<0>(values);
    const std::string& secondPart = std::get<1>(values);
    const int interruptedAtCount = std::get<2>(values);

    GIVEN("An incomplete XML snippet with " + std::to_string(firstPart.size()) + " characters") {
      WHEN("Starting to parse an incomplete XML, setting a savePoint, Reading data out of the document until "
           "the end of the snippet is reachen and then repeating the deserialization after the last save point")
      {
        std::unique_ptr<sergut::xml::PullParser> parserTmp = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(firstPart));
        sergut::xml::PullParser& parser = *parserTmp;
        CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenDocument);
        CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
        CHECK(parser.getCurrentTagName() == std::string("root"));
        CHECK(parser.parseNext() == sergut::xml::ParseTokenType::OpenTag);
        CHECK(parser.getCurrentTagName() == std::string("inner"));
        THEN("The no error is detected by the parser") {
          int i = 0;
          for(; i < interruptedAtCount; ++i) {
            parser.setSavePointAtCurrentTag();
            const SavepointTest data = sergut::XmlDeserializer::deserializeFromSnippet<SavepointTest>("inner", parser);
            CHECK(data == (SavepointTest{1, 1}));
          }
          parser.setSavePointAtCurrentTag();
          CHECK_THROWS_AS(sergut::XmlDeserializer::deserializeFromSnippet<SavepointTest>("inner", parser), sergut::ParsingException);
          parser.restoreToSavePoint();
          parser.appendData(secondPart.data(), secondPart.size());
          const SavepointTest data = sergut::XmlDeserializer::deserializeFromSnippet<SavepointTest>("inner", parser);
          CHECK(data == (SavepointTest{1, 1}));
          ++i;
          for(; i < 2; ++i) {
            parser.setSavePointAtCurrentTag();
            const SavepointTest data = sergut::XmlDeserializer::deserializeFromSnippet<SavepointTest>("inner", parser);
            CHECK(data == (SavepointTest{1, 1}));
          }
        }
      }
    }
  }
}


/*
 *  TODO:
 * * UTF-16 handling
 */
static const char utf16Xml[] = {
  static_cast<char>(0xff), static_cast<char>(0xfe),
              0x3c, 0x00, 0x44, 0x00, 0x75, 0x00, 0x6d, 0x00, 0x6d, 0x00,
  0x79, 0x00, 0x20, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x4d, 0x00,
  0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00,
  0x3d, 0x00, 0x22, 0x00, 0x32, 0x00, 0x31, 0x00, 0x22, 0x00, 0x20, 0x00,
  0x69, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00,
  0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x32, 0x00, 0x3d, 0x00, 0x22, 0x00,
  0x39, 0x00, 0x39, 0x00, 0x22, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x69, 0x00,
  0x6e, 0x00, 0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00,
  0x65, 0x00, 0x72, 0x00, 0x33, 0x00, 0x3e, 0x00, 0x31, 0x00, 0x32, 0x00,
  0x34, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x74, 0x00,
  0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00,
  0x33, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x63, 0x00, 0x68, 0x00, 0x69, 0x00,
  0x6c, 0x00, 0x64, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00,
  0x65, 0x00, 0x72, 0x00, 0x34, 0x00, 0x20, 0x00, 0x69, 0x00, 0x6e, 0x00,
  0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00,
  0x72, 0x00, 0x31, 0x00, 0x3d, 0x00, 0x22, 0x00, 0x2d, 0x00, 0x32, 0x00,
  0x37, 0x00, 0x22, 0x00, 0x20, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x74, 0x00,
  0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00,
  0x32, 0x00, 0x3d, 0x00, 0x22, 0x00, 0x2d, 0x00, 0x34, 0x00, 0x32, 0x00,
  0x22, 0x00, 0x20, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x65, 0x00,
  0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00,
  0x33, 0x00, 0x3d, 0x00, 0x22, 0x00, 0x34, 0x00, 0x3a, 0x00, 0x34, 0x00,
  0x35, 0x00, 0x3a, 0x00, 0x30, 0x00, 0x30, 0x00, 0x22, 0x00, 0x20, 0x00,
  0x69, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00,
  0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x34, 0x00, 0x3d, 0x00, 0x22, 0x00,
  0x2d, 0x00, 0x32, 0x00, 0x33, 0x00, 0x22, 0x00, 0x20, 0x00, 0x64, 0x00,
  0x6f, 0x00, 0x75, 0x00, 0x62, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x4d, 0x00,
  0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x35, 0x00,
  0x3d, 0x00, 0x22, 0x00, 0x33, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x34, 0x00,
  0x31, 0x00, 0x35, 0x00, 0x39, 0x00, 0x22, 0x00, 0x20, 0x00, 0x66, 0x00,
  0x6c, 0x00, 0x6f, 0x00, 0x61, 0x00, 0x74, 0x00, 0x4d, 0x00, 0x65, 0x00,
  0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x36, 0x00, 0x3d, 0x00,
  0x22, 0x00, 0x32, 0x00, 0x2e, 0x00, 0x37, 0x00, 0x31, 0x00, 0x38, 0x00,
  0x22, 0x00, 0x3e, 0x00, 0x2d, 0x00, 0x31, 0x00, 0x32, 0x00, 0x37, 0x00,
  0x3c, 0x00, 0x2f, 0x00, 0x63, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6c, 0x00,
  0x64, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00,
  0x72, 0x00, 0x34, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x69, 0x00, 0x6e, 0x00,
  0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00,
  0x72, 0x00, 0x35, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x6e, 0x00, 0x65, 0x00,
  0x73, 0x00, 0x74, 0x00, 0x65, 0x00, 0x64, 0x00, 0x49, 0x00, 0x6e, 0x00,
  0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00,
  0x72, 0x00, 0x35, 0x00, 0x3e, 0x00, 0x36, 0x00, 0x35, 0x00, 0x30, 0x00,
  0x30, 0x00, 0x30, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x6e, 0x00, 0x65, 0x00,
  0x73, 0x00, 0x74, 0x00, 0x65, 0x00, 0x64, 0x00, 0x49, 0x00, 0x6e, 0x00,
  0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00,
  0x72, 0x00, 0x35, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x69, 0x00,
  0x6e, 0x00, 0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00,
  0x65, 0x00, 0x72, 0x00, 0x35, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x69, 0x00,
  0x6e, 0x00, 0x74, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00,
  0x65, 0x00, 0x72, 0x00, 0x36, 0x00, 0x3e, 0x00, 0x32, 0x00, 0x35, 0x00,
  0x35, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x74, 0x00,
  0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00,
  0x36, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x73, 0x00, 0x74, 0x00, 0x72, 0x00,
  0x69, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00,
  0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x37, 0x00, 0x3e, 0x00, 0x0a, 0x00,
  0x73, 0x00, 0x74, 0x00, 0x72, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x67, 0x00,
  0x5c, 0x00, 0x65, 0x00, 0x73, 0x00, 0x63, 0x00, 0x61, 0x00, 0x70, 0x00,
  0x65, 0x00, 0x64, 0x00, 0x26, 0x00, 0x71, 0x00, 0x75, 0x00, 0x6f, 0x00,
  0x74, 0x00, 0x3b, 0x00, 0x71, 0x00, 0x75, 0x00, 0x6f, 0x00, 0x74, 0x00,
  0x65, 0x00, 0x64, 0x00, 0x26, 0x00, 0x71, 0x00, 0x75, 0x00, 0x6f, 0x00,
  0x74, 0x00, 0x3b, 0x00, 0x20, 0x00, 0x26, 0x00, 0x6c, 0x00, 0x74, 0x00,
  0x3b, 0x00, 0x62, 0x00, 0x26, 0x00, 0x67, 0x00, 0x74, 0x00, 0x3b, 0x00,
  0x44, 0x00, 0x61, 0x00, 0x74, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x26, 0x00,
  0x6c, 0x00, 0x74, 0x00, 0x3b, 0x00, 0x2f, 0x00, 0x62, 0x00, 0x26, 0x00,
  0x67, 0x00, 0x74, 0x00, 0x3b, 0x00, 0x66, 0x00, 0x6f, 0x00, 0x6f, 0x00,
  0x3c, 0x00, 0x2f, 0x00, 0x73, 0x00, 0x74, 0x00, 0x72, 0x00, 0x69, 0x00,
  0x6e, 0x00, 0x67, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00,
  0x65, 0x00, 0x72, 0x00, 0x37, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x63, 0x00,
  0x68, 0x00, 0x61, 0x00, 0x72, 0x00, 0x50, 0x00, 0x74, 0x00, 0x72, 0x00,
  0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00,
  0x38, 0x00, 0x3e, 0x00, 0x63, 0x00, 0x68, 0x00, 0x61, 0x00, 0x72, 0x00,
  0x2a, 0x00, 0x20, 0x00, 0x44, 0x00, 0x61, 0x00, 0x74, 0x00, 0x65, 0x00,
  0x6e, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x63, 0x00, 0x68, 0x00, 0x61, 0x00,
  0x72, 0x00, 0x50, 0x00, 0x74, 0x00, 0x72, 0x00, 0x4d, 0x00, 0x65, 0x00,
  0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x38, 0x00, 0x3e, 0x00,
  0x3c, 0x00, 0x63, 0x00, 0x68, 0x00, 0x61, 0x00, 0x72, 0x00, 0x4d, 0x00,
  0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x39, 0x00,
  0x3e, 0x00, 0x63, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x63, 0x00, 0x68, 0x00,
  0x61, 0x00, 0x72, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00,
  0x65, 0x00, 0x72, 0x00, 0x39, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x63, 0x00,
  0x68, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x64, 0x00, 0x56, 0x00, 0x65, 0x00,
  0x63, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x4d, 0x00, 0x65, 0x00,
  0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00, 0x30, 0x00,
  0x20, 0x00, 0x67, 0x00, 0x72, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x64, 0x00,
  0x43, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x64, 0x00, 0x56, 0x00,
  0x61, 0x00, 0x6c, 0x00, 0x75, 0x00, 0x65, 0x00, 0x3d, 0x00, 0x22, 0x00,
  0x32, 0x00, 0x32, 0x00, 0x22, 0x00, 0x2f, 0x00, 0x3e, 0x00, 0x3c, 0x00,
  0x63, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x64, 0x00, 0x56, 0x00,
  0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x4d, 0x00,
  0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00,
  0x30, 0x00, 0x20, 0x00, 0x67, 0x00, 0x72, 0x00, 0x61, 0x00, 0x6e, 0x00,
  0x64, 0x00, 0x43, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x64, 0x00,
  0x56, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x75, 0x00, 0x65, 0x00, 0x3d, 0x00,
  0x22, 0x00, 0x33, 0x00, 0x33, 0x00, 0x22, 0x00, 0x2f, 0x00, 0x3e, 0x00,
  0x3c, 0x00, 0x63, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x64, 0x00,
  0x56, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00,
  0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00,
  0x31, 0x00, 0x30, 0x00, 0x20, 0x00, 0x67, 0x00, 0x72, 0x00, 0x61, 0x00,
  0x6e, 0x00, 0x64, 0x00, 0x43, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6c, 0x00,
  0x64, 0x00, 0x56, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x75, 0x00, 0x65, 0x00,
  0x3d, 0x00, 0x22, 0x00, 0x34, 0x00, 0x34, 0x00, 0x22, 0x00, 0x2f, 0x00,
  0x3e, 0x00, 0x3c, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x56, 0x00,
  0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x4d, 0x00,
  0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00,
  0x31, 0x00, 0x3e, 0x00, 0x31, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x69, 0x00,
  0x6e, 0x00, 0x74, 0x00, 0x56, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00,
  0x6f, 0x00, 0x72, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00,
  0x65, 0x00, 0x72, 0x00, 0x31, 0x00, 0x31, 0x00, 0x3e, 0x00, 0x3c, 0x00,
  0x69, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x56, 0x00, 0x65, 0x00, 0x63, 0x00,
  0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00,
  0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00, 0x31, 0x00, 0x3e, 0x00,
  0x32, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x74, 0x00,
  0x56, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00,
  0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00,
  0x31, 0x00, 0x31, 0x00, 0x3e, 0x00, 0x3c, 0x00, 0x69, 0x00, 0x6e, 0x00,
  0x74, 0x00, 0x56, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6f, 0x00,
  0x72, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00,
  0x72, 0x00, 0x31, 0x00, 0x31, 0x00, 0x3e, 0x00, 0x33, 0x00, 0x3c, 0x00,
  0x2f, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x56, 0x00, 0x65, 0x00,
  0x63, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x4d, 0x00, 0x65, 0x00,
  0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00, 0x31, 0x00,
  0x3e, 0x00, 0x3c, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x56, 0x00,
  0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x4d, 0x00,
  0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00,
  0x31, 0x00, 0x3e, 0x00, 0x34, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x69, 0x00,
  0x6e, 0x00, 0x74, 0x00, 0x56, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00,
  0x6f, 0x00, 0x72, 0x00, 0x4d, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00,
  0x65, 0x00, 0x72, 0x00, 0x31, 0x00, 0x31, 0x00, 0x3e, 0x00, 0x3c, 0x00,
  0x63, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x64, 0x00, 0x4d, 0x00,
  0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x65, 0x00, 0x72, 0x00, 0x31, 0x00,
  0x32, 0x00, 0x20, 0x00, 0x67, 0x00, 0x72, 0x00, 0x61, 0x00, 0x6e, 0x00,
  0x64, 0x00, 0x43, 0x00, 0x68, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x64, 0x00,
  0x56, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x75, 0x00, 0x65, 0x00, 0x3d, 0x00,
  0x22, 0x00, 0x2d, 0x00, 0x39, 0x00, 0x39, 0x00, 0x22, 0x00, 0x2f, 0x00,
  0x3e, 0x00, 0x3c, 0x00, 0x2f, 0x00, 0x44, 0x00, 0x75, 0x00, 0x6d, 0x00,
  0x6d, 0x00, 0x79, 0x00, 0x3e, 0x00, 0x0a, 0x00
};

//{"intMember1":21,"intMember2":99,"intMember3":124,"childMember4":{"intMember1":-27,"intMember2":-42,"timeMember3":"4:45:00","intMember4":-23,"doubleMember5":3.14159,"floatMember6":2.718,"intMember7":-127},"intMember5":{"nestedIntMember5":65000},"intMember6":255,"stringMember7":"\u000fstring\\escaped\"quoted\" <b>Daten<\/b>foo","charPtrMember8":"char* Daten","charMember9":"c","childVectorMember10":[{"grandChildValue":22},{"grandChildValue":33},{"grandChildValue":44}],"intVectorMember11":[1,2,3,4],"childMember12":{"grandChildValue":-99}}


//res: <Dummy intMember1="21" intMember2="99"><intMember3>124</intMember3><childMember4 intMember1="-27" intMember2="-42" timeMember3="4:45:00" intMember4="-23" doubleMember5="3.14159" floatMember6="2.718">-127</childMember4><intMember5><nestedIntMember5>65000</nestedIntMember5></intMember5><intMember6>255</intMember6><stringMember7>string\escaped&quot;quoted&quot; &lt;b&gt;Daten&lt;/b&gt;foo</stringMember7><charPtrMember8>char* Daten</charPtrMember8><charMember9>c</charMember9><childVectorMember10 grandChildValue="22"/><childVectorMember10 grandChildValue="33"/><childVectorMember10 grandChildValue="44"/><intVectorMember11>1</intVectorMember11><intVectorMember11>2</intVectorMember11><intVectorMember11>3</intVectorMember11><intVectorMember11>4</intVectorMember11><childMember12 grandChildValue="-99"/></Dummy>
//res2: <Dummy intMember1="21" intMember2="99"><intMember3>124</intMember3><childMember4 intMember1="-27" intMember2="-42" timeMember3="4:45:00" intMember4="-23" doubleMember5="3.14159" floatMember6="2.718">-127</childMember4><intMember5><nestedIntMember5>65000</nestedIntMember5></intMember5><intMember6>255</intMember6><stringMember7>string\escaped&quot;quoted&quot; &lt;b&gt;Daten&lt;/b&gt;foo</stringMember7><charPtrMember8>char* Daten</charPtrMember8><charMember9>c</charMember9><childVectorMember10 grandChildValue="22"/><childVectorMember10 grandChildValue="33"/><childVectorMember10 grandChildValue="44"/><intVectorMember11>1</intVectorMember11><intVectorMember11>2</intVectorMember11><intVectorMember11>3</intVectorMember11><intVectorMember11>4</intVectorMember11><childMember12 grandChildValue="-99"/></Dummy>
//Orig:  <Dummy double2="2.345" int1="12345" time3="3:23:99"><uchar5>21</uchar5><char4><nestedChar4>X</nestedChar4></char4><time6><nestedTime6>12:34:55</nestedTime6></time6></Dummy>
//Later: <Dummy int1="12345" double2="2.345" time3="3:23:99"><char4><nestedChar4>X</nestedChar4></char4><uchar5>21</uchar5><time6><nestedTime6>12:34:55</nestedTime6></time6></Dummy>
//Orig:  <vector>1</vector><vector>2</vector>
//Later: <vector>1</vector><vector>2</vector>
//Orig:  <vector>1:00:03</vector><vector>20:34:35</vector>
//Later: <vector>1:00:03</vector><vector>20:34:35</vector>


//struct Ser_char {
//  char value;
//};
//SERGUT_FUNCTION(Ser_char, data, ar)
//{
//  ar & SERGUT_MMEMBER(data, value);
//}
//TEST_CASE("Serialize datatype char as XML", "[sergut]")
//{
//  Ser_char data{ 'a' };
//  const std::string expectedResult = "<char value=\"a\"></char>";
//  sergut::XmlSerializer ser;
//  ser.serializeData("char", data);
//  CHECK(ser.str() == expectedResult);
//}

TEST_CASE("Serialize complex class", "[sergut]")
{

  GIVEN("A complex C++ POD datastructure")  {
    const TestParent tp{ 21, 99, 124, TestChild{ -27, -42, Time{4, 45}, -23, 3.14159, 2.718, -127 }, 65000, 255,
                         "\nstring\\escaped\"quoted\" &<b>Daten</b>foo", "char* Daten", 'c', { {22}, {33}, {44} }, { 1, 2, 3, 4}, { -99 } };

    WHEN("The datastructure is serialized to XML") {
      sergut::XmlSerializer ser;
      ser.serializeData("Dummy", tp);

      THEN("The result is the specified string") {
        const std::string req =
            "<Dummy intMember1=\"21\" intMember2=\"99\"><intMember3>124</intMember3>"
            "<childMember4 intMember1=\"-27\" intMember2=\"-42\" timeMember3=\"4:45:00\" intMember4=\"-23\" doubleMember5=\"3.14159\" "
              "floatMember6=\"2.718\">-127</childMember4>"
            "<intMember5><nestedIntMember5>65000</nestedIntMember5></intMember5>"
            "<intMember6>255</intMember6><stringMember7>\nstring\\escaped&quot;quoted&quot; &amp;&lt;b&gt;Daten&lt;/b&gt;foo</stringMember7>"
            "<charPtrMember8>char* Daten</charPtrMember8><charMember9>c</charMember9><childVectorMember10 grandChildValue=\"22\"/>"
            "<childVectorMember10 grandChildValue=\"33\"/><childVectorMember10 grandChildValue=\"44\"/>"
            "<intVectorMember11>1</intVectorMember11><intVectorMember11>2</intVectorMember11><intVectorMember11>3</intVectorMember11>"
            "<intVectorMember11>4</intVectorMember11><childMember12 grandChildValue=\"-99\"/></Dummy>";
        CHECK(ser.str() == req);
      }
    }
    WHEN("The datastructure is serialized to XML, then deserialized from XML, and serialized again") {
      THEN("The two serializations are equal using XmlDeserializer") {
        sergut::XmlSerializer ser;
        ser.serializeData("Dummy", tp);
        const std::string xmlResult = ser.str();
        sergut::XmlDeserializer deser{sergut::misc::ConstStringRef(xmlResult)};
        const TestParent tpDeser = deser.deserializeData<TestParent>("Dummy");
        sergut::XmlSerializer ser2;
        ser2.serializeData("Dummy", tp);
        CHECK(ser.str() == ser2.str());
      }
      THEN("The two serializations are equal using XmlDeserializerTiny") {
        sergut::XmlSerializer ser;
        ser.serializeData("Dummy", tp);
        sergut::XmlDeserializerTiny deser(ser.str());
        const TestParent tpDeser = deser.deserializeData<TestParent>("Dummy");
        sergut::XmlSerializer ser2;
        ser2.serializeData("Dummy", tp);
        CHECK(ser.str() == ser2.str());
      }
      THEN("The two serializations are equal using XmlDeserializerTiny2") {
        sergut::XmlSerializer ser;
        ser.serializeData("Dummy", tp);
        sergut::XmlDeserializerTiny2 deser(ser.str());
        const TestParent tpDeser = deser.deserializeData<TestParent>("Dummy");
        sergut::XmlSerializer ser2;
        ser2.serializeData("Dummy", tp);
        CHECK(ser.str() == ser2.str());
      }
    }
  }
}


TEST_CASE("Deserialize XML into a simple class", "[sergut]")
{
  GIVEN("An XML-string and a simple class with members in wrong Order") {
    const std::string origXml("<Dummy double2=\"2.345\" int1=\"12345\" time3=\"3:23:99\"><uchar5>21</uchar5>"
                              "<char4><nestedChar4>X</nestedChar4></char4>"
                              "<time6><nestedTime6>12:34:55</nestedTime6></time6></Dummy>");
    const Simple origVal{ 12345, 2.345, Time{3, 23, 99}, 'X', 21, Time{12, 34, 55}};
    WHEN("The XML-string is serialized into the simple class (XmlDeserializer)") {
      sergut::XmlDeserializer deser{sergut::misc::ConstStringRef(origXml)};
      const Simple res = deser.deserializeData<Simple>("Dummy");
      CHECK(res == origVal);
    }
    WHEN("The XML-string is serialized into the simple class (XmlDeserializerTiny)") {
      sergut::XmlDeserializerTiny deser(origXml);
      const Simple res = deser.deserializeData<Simple>("Dummy");
      CHECK(res == origVal);
    }
    WHEN("The XML-string is serialized into the simple class (XmlDeserializerTiny2)") {
      sergut::XmlDeserializerTiny2 deser(origXml);
      const Simple res = deser.deserializeData<Simple>("Dummy");
      CHECK(res == origVal);
    }
  }
}

struct TestFlexibleXml {
public:
  bool operator==(const TestFlexibleXml& rhs) const {
    return intVal       == rhs.intVal
        && simpleVec    == rhs.simpleVec
        && nestedIntVec == rhs.nestedIntVec
        && emptyVec     == rhs.emptyVec
        && nestedString == rhs.nestedString;
  }

public:
  int intVal;
  std::vector<Simple> simpleVec;
  std::vector<int> nestedIntVec;
  std::vector<std::string> emptyVec;
  std::string nestedString;
  std::vector<std::string> emptyNestedVector;
};
SERGUT_FUNCTION(TestFlexibleXml, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, intVal)
      & sergut::children
      & SERGUT_MMEMBER(data, simpleVec)
      & SERGUT_NESTED_MMEMBER(data, nestedIntVec, val)
      & SERGUT_MMEMBER(data, emptyVec)
      & SERGUT_NESTED_MMEMBER(data, nestedString, val)
      & SERGUT_NESTED_MMEMBER(data, emptyNestedVector, val)
      ;
}

TEST_CASE("Deserialize XML with different spaces", "[sergut]")
{
  const TestFlexibleXml expectedResult{ 1970,
                                        { { 12345, 2.345, Time{3, 23, 99}, 'X', 21, Time{12, 34, 55} },
                                          { 1,    -2.345, Time{0,  0,  1}, 'Y', 12, Time{ 1,  2,  3} } },
                                        { -12, 33 },
                                        { },
                                        "nestedString",
                                        { } };


  GIVEN("An XML-string without additional spaces") {
    std::string xml("<outerName intVal='1970'>"
                    "<simpleVec int1='12345' double2='2.345' time3='3:23:99'><char4><nestedChar4>X</nestedChar4></char4>"
                    "<uchar5>21</uchar5><time6><nestedTime6>12:34:55</nestedTime6></time6></simpleVec>"
                    "<simpleVec int1='1' double2='-2.345' time3='0:00:01'><char4><nestedChar4>Y</nestedChar4></char4>"
                    "<uchar5>12</uchar5><time6><nestedTime6>1:02:03</nestedTime6></time6></simpleVec>"
                    "<nestedIntVec><val>-12</val><val>33</val></nestedIntVec>"
                    "<nestedString><val>nestedString</val></nestedString>"
                    "<emptyNestedVector/>"
                    "</outerName>");
    WHEN("Deserializing it into a datastructure") {
      sergut::XmlDeserializer ser(xml);
      THEN("it should yield the expected result") {
        CHECK(ser.deserializeData<TestFlexibleXml>("outerName") == expectedResult);
      }
    }
  }


  GIVEN("A nested XML-string without additional spaces") {
    std::string xml("<outerouter><outerName intVal='1970'>"
                    "<simpleVec int1='12345' double2='2.345' time3='3:23:99'><char4><nestedChar4>X</nestedChar4></char4>"
                    "<uchar5>21</uchar5><time6><nestedTime6>12:34:55</nestedTime6></time6></simpleVec>"
                    "<simpleVec int1='1' double2='-2.345' time3='0:00:01'><char4><nestedChar4>Y</nestedChar4></char4>"
                    "<uchar5>12</uchar5><time6><nestedTime6>1:02:03</nestedTime6></time6></simpleVec>"
                    "<nestedIntVec><val>-12</val><val>33</val></nestedIntVec>"
                    "<nestedString><val>nestedString</val></nestedString>"
                    "<emptyNestedVector/>"
                    "</outerName></outerouter>");
    WHEN("Deserializing it into a datastructure") {
      sergut::XmlDeserializer ser(xml);
      THEN("it should yield the expected result") {
        CHECK(ser.deserializeNestedData<TestFlexibleXml>("outerouter", "outerName") == expectedResult);
      }
    }
  }

  GIVEN("An XML-string with additional spaces") {
    std::string xml("<outerName  intVal = '1970' >\n"
                    "<simpleVec int1 = '12345' double2 = '2.345' time3 = '3:23:99' > <char4 > <nestedChar4 >X</nestedChar4 > </char4 >\n"
                    "<uchar5>21</uchar5> <time6> <nestedTime6>12:34:55</nestedTime6> </time6> </simpleVec>\n"
                    "<simpleVec int1 = '1' double2 = '-2.345' time3 = '0:00:01'> <char4> <nestedChar4>Y</nestedChar4> </char4>\n"
                    "<uchar5>12</uchar5> <time6> <nestedTime6>1:02:03</nestedTime6> </time6> </simpleVec>\n"
                    "<nestedIntVec> <val>-12</val> <val>33</val> </nestedIntVec>\n"
                    "<nestedString> <val>nestedString</val> </nestedString>\n"
                    "<emptyNestedVector />\n"
                    "</outerName>\n");
    WHEN("Deserializing it into a datastructure") {
      sergut::XmlDeserializer ser(xml);
      THEN("it should yield the expected result") {
        CHECK(ser.deserializeData<TestFlexibleXml>("outerName") == expectedResult);
      }
    }
  }

  GIVEN("An XML-string with additional spaces") {
    std::string xml("<outerouter><outerName  intVal = '1970' >\n"
                    "<simpleVec int1 = '12345' double2 = '2.345' time3 = '3:23:99' > <char4 > <nestedChar4 >X</nestedChar4 > </char4 >\n"
                    "<uchar5>21</uchar5> <time6> <nestedTime6>12:34:55</nestedTime6> </time6> </simpleVec>\n"
                    "<simpleVec int1 = '1' double2 = '-2.345' time3 = '0:00:01'> <char4> <nestedChar4>Y</nestedChar4> </char4>\n"
                    "<uchar5>12</uchar5> <time6> <nestedTime6>1:02:03</nestedTime6> </time6> </simpleVec>\n"
                    "<nestedIntVec> <val>-12</val> <val>33</val> </nestedIntVec>\n"
                    "<nestedString> <val>nestedString</val> </nestedString>\n"
                    "<emptyNestedVector />\n"
                    "</outerName></outerouter>\n");
    WHEN("Deserializing it into a datastructure") {
      sergut::XmlDeserializer ser(xml);
      THEN("it should yield the expected result") {
        CHECK(ser.deserializeNestedData<TestFlexibleXml>("outerouter", "outerName") == expectedResult);
      }
    }
  }
}



//int main_disabled(int /*argc*/, char */*argv*/[])
//{
//  const TestParent tp{ 21, 99, 124, TestChild{ -27, -42, {4, 45}, -23, 3.14159, 2.718, -127 }, 65000, 255,
//                       "\x0fstring\\escaped\"quoted\" <b>Daten</b>foo", "char* Daten", 'c', { {22}, {33}, {44} }, { 1, 2, 3, 4}, { -99 } };

//  {
//    sergut::JsonSerializer ser;
//    ser.serializeData("Dummy", tp);
//    std::cout << ser.str() << std::endl;
//  }
//  std::cout << "\n\n";
//  {



//    const std::string res = [&]() {
//      sergut::XmlSerializer ser;
//      ser.serializeData("Dummy", tp);
//      return ser.str();
//    }();
//    std::cout << "res: " << res << std::endl;
//    sergut::XmlDeserializer deser{sergut::misc::ConstStringRef(res)};
//    const TestParent tpDeser = deser.deserializeData<TestParent>("Dummy");
//    const std::string res2 = [&]() {
//      sergut::XmlSerializer ser;
//      ser.serializeData("Dummy", tpDeser);
//      return ser.str();
//    }();
//    std::cout << "res2: " << res2 << std::endl;
//  }

//  {
//    const std::string origXml("<Dummy double2=\"2.345\" int1=\"12345\" time3=\"3:23:99\"><uchar5>21</uchar5><char4><nestedChar4>X</nestedChar4></char4><time6><nestedTime6>12:34:55</nestedTime6></time6></Dummy>");
//    sergut::XmlDeserializer deser{sergut::misc::ConstStringRef(origXml)};
//    const Simple res = deser.deserializeData<Simple>("Dummy");
//    sergut::XmlSerializer ser;
//    ser.serializeData("Dummy", res);
//    std::cout << "Orig:  " << origXml << "\n"
//              << "Later: " << ser.str() << "\n";
//  }

////  // unsupported
////  {
////    const std::string origXml("<vector>1</vector><vector>2</vector>");
////    sergut::XmlDeserializer deser(origXml);
////    const std::vector<int> res = deser.deserializeData<std::vector<int>>("vector", sergut::XmlValueType::Child);
////    sergut::XmlSerializer ser;
////    ser.serializeData("vector", res);
////    std::cout << "Orig:  " << origXml << "\n"
////              << "Later: " << ser.str() << "\n";
////  }

//  // unsupported
////  {
////    const std::string origXml("<vector>1:00:03</vector><vector>20:34:35</vector>");
////    sergut::XmlDeserializer deser(origXml);
////    const std::vector<Time> res =
////        deser.deserializeData<std::vector<Time>>("vector", sergut::XmlValueType::Child);
////    sergut::XmlSerializer ser;
////    ser.serializeData<std::vector<Time>>("vector", res);
////    std::cout << "Orig:  " << origXml << "\n"
////              << "Later: " << ser.str() << "\n";
////  }

////  {
////    sergut::XmlDeserializer deser(utf16Xml);
////    const TestParent tpDeser = deser.deserializeData<TestParent>("Dummy");
////    const std::string res2 = [&]() {
////      sergut::XmlSerializer ser;
////      ser.serializeData("Dummy", tpDeser);
////      return ser.str();
////    }();
////    std::cout << "res2: " << res2 << std::endl;
////  }
//  return 0;
//}
