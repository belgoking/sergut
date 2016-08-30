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

#include "TestSupportClasses.h"

#include "sergut/JsonSerializer.h"
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
// Test serialization/desirialization of Data
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
} \

#if 0
// TODO extend TinyDeserializers to support SingleChild plain members
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

#endif

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
    const type tpDeser1 = deser.deserializeNestedData<type>(#type, "value", sergut::XmlValueType::Attribute); \
    CHECK(tpDeser1 == datatypeValue); \
  } \
} \

#if 0
// TODO extend TinyDeserializers to support SingleChild plain members
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

#endif

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
    const type tpDeser1 = deser.deserializeNestedData<type>(#type, "value", sergut::XmlValueType::Child); \
    CHECK(tpDeser1 == datatypeValue); \
  } \
} \

#if 0
// TODO extend TinyDeserializers to support SingleChild plain members
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

#endif

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
} \

#if 0
// TODO extend TinyDeserializers to support SingleChild plain members
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

#endif

DEFINE_BASIC_DATATYPE_AS_SINGLE_CHILD_TEST(char,        'a', "<char>a</char>")
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


#define DEFINE_VECTOR_TEST(type, datatypeValue, expectedResult) \
struct Ser_VectorOf ## type { \
  std::vector<type> value; \
  bool operator==(const Ser_VectorOf ## type& rhs) const { return value == rhs.value; } \
}; \
SERGUT_FUNCTION(Ser_VectorOf ## type, data, ar) \
{ \
  ar & sergut::children & SERGUT_MMEMBER(data, value); \
} \
TEST_CASE("DeSerialize vector of " #type " as XML", "[sergut]") \
{ \
  SECTION("Serialize") { \
    Ser_VectorOf ## type data{ datatypeValue }; \
    sergut::XmlSerializer ser; \
    ser.serializeData("VectorOf" #type, data); \
    CHECK(ser.str() == expectedResult); \
  } \
  SECTION("Deserialize XmlDeserializer") { \
    sergut::XmlDeserializer deser(expectedResult); \
    const Ser_VectorOf ## type tpDeser1 = deser.deserializeData<Ser_VectorOf ## type>("VectorOf" #type); \
    CHECK(tpDeser1 == Ser_VectorOf ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny") { \
  sergut::XmlDeserializerTiny deser(expectedResult); \
  const Ser_VectorOf ## type tpDeser2 = deser.deserializeData<Ser_VectorOf ## type>("VectorOf" #type); \
  CHECK(tpDeser2 == Ser_VectorOf ## type{ datatypeValue }); \
  } \
  SECTION("Deserialize XmlDeserializerTiny2") { \
  sergut::XmlDeserializerTiny2 deser(expectedResult); \
  const Ser_VectorOf ## type tpDeser2 = deser.deserializeData<Ser_VectorOf ## type>("VectorOf" #type); \
  CHECK(tpDeser2 == Ser_VectorOf ## type{ datatypeValue }); \
  } \
} \

DEFINE_VECTOR_TEST(char,     (std::vector<char>{'a', 'b', 'c'}), "<VectorOfchar><value>a</value><value>b</value><value>c</value></VectorOfchar>")
DEFINE_VECTOR_TEST(uint8_t,  (std::vector<uint8_t>{0, 17, 255}), "<VectorOfuint8_t><value>0</value><value>17</value><value>255</value></VectorOfuint8_t>")
DEFINE_VECTOR_TEST(int16_t,  (std::vector<int16_t>{-32768, 17, 32767}), "<VectorOfint16_t><value>-32768</value><value>17</value><value>32767</value></VectorOfint16_t>")
DEFINE_VECTOR_TEST(uint16_t, (std::vector<uint16_t>{0, 17, 65535}), "<VectorOfuint16_t><value>0</value><value>17</value><value>65535</value></VectorOfuint16_t>")
DEFINE_VECTOR_TEST(int32_t,  (std::vector<int32_t>{-2147483648, 17, 2147483647}), "<VectorOfint32_t><value>-2147483648</value><value>17</value><value>2147483647</value></VectorOfint32_t>")
DEFINE_VECTOR_TEST(uint32_t, (std::vector<uint32_t>{0, 17, 4294967295}), "<VectorOfuint32_t><value>0</value><value>17</value><value>4294967295</value></VectorOfuint32_t>")
DEFINE_VECTOR_TEST(int64_t,  (std::vector<int64_t>{-9223372036854775807LL, 17, 9223372036854775807LL}), "<VectorOfint64_t><value>-9223372036854775807</value><value>17</value><value>9223372036854775807</value></VectorOfint64_t>")
DEFINE_VECTOR_TEST(uint64_t, (std::vector<uint64_t>{0, 17, 9223372036854775808ULL}), "<VectorOfuint64_t><value>0</value><value>17</value><value>9223372036854775808</value></VectorOfuint64_t>")
DEFINE_VECTOR_TEST(float,    (std::vector<float>{0.0, -17.25, 0.0000025}), "<VectorOffloat><value>0</value><value>-17.25</value><value>2.5e-06</value></VectorOffloat>")
DEFINE_VECTOR_TEST(double,   (std::vector<double>{0.0, -17.25, 0.0000025}), "<VectorOfdouble><value>0</value><value>-17.25</value><value>2.5e-06</value></VectorOfdouble>")
DEFINE_VECTOR_TEST(Time,     (std::vector<Time>{{14,34,15}, {15,55,55}, {0,0,0}}), "<VectorOfTime><value>14:34:15</value><value>15:55:55</value><value>0:00:00</value></VectorOfTime>")
namespace {
typedef std::string string;
DEFINE_VECTOR_TEST(string, (std::vector<string>{"hallo", "liebe", "Welt"}), "<VectorOfstring><value>hallo</value><value>liebe</value><value>Welt</value></VectorOfstring>")
}
namespace {
typedef std::string escapedString;
DEFINE_VECTOR_TEST(escapedString, (std::vector<escapedString>{"<b>\"STRING&amp;\"</b>", "<b>\"STRING&uuml;\"</b>"}),
                   "<VectorOfescapedString><value>&lt;b&gt;&quot;STRING&amp;amp;&quot;&lt;/b&gt;</value><value>&lt;b&gt;&quot;STRING&amp;uuml;&quot;&lt;/b&gt;</value></VectorOfescapedString>")
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
DEFINE_NESTED_VECTOR_TEST(Time,     (std::vector<Time>{{15,45,45},{0,0,0},{23,59,59}}), "<NestedVectorOfTime><values><value>15:45:45</value><value>0:00:00</value><value>23:59:59</value></values></NestedVectorOfTime>")
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
  ar & sergut::children & Archive::toNamedMember("outerNested", Archive::toNamedMember("innerNested", Archive::toNamedMember("value", data.value, true), true), true); \
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


class TestChild2
{
public:
  TestChild2(int pGrandChildValue = 0) : grandChildValue(pGrandChildValue) { }
private:
  SERGUT_FUNCTION_FRIEND_DECL(TestChild2, data, ar);
  int grandChildValue;
};

SERGUT_FUNCTION(TestChild2, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, grandChildValue);
}

struct TestChild
{
  long long intMember1;
  long intMember2;
  Time timeMember3;
  int intMember4;
  double doubleMember5;
  float floatMember6;
  short intMember7;
};

SERGUT_FUNCTION(TestChild, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, intMember1)
      & SERGUT_MMEMBER(data, intMember2)
      & SERGUT_MMEMBER(data, timeMember3)
      & SERGUT_MMEMBER(data, intMember4)
      & SERGUT_MMEMBER(data, doubleMember5)
      & SERGUT_MMEMBER(data, floatMember6)
      & sergut::plainChild
      & SERGUT_MMEMBER(data, intMember7);
}

struct TestParent
{
  unsigned long long intMember1;
  unsigned long intMember2;
  unsigned int intMember3;
  TestChild childMember4;
  unsigned short intMember5;
  unsigned char intMember6;
  std::string stringMember7;
  std::string charPtrMember8;
  char charMember9;
  std::vector<TestChild2> childVectorMember10;
  std::vector<int> intVectorMember11;
  TestChild2 childMember12;
};

SERGUT_FUNCTION(TestParent, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, intMember1)
      & SERGUT_MMEMBER(data, intMember2)
      & sergut::children
      & SERGUT_MMEMBER(data, intMember3)
      & SERGUT_MMEMBER(data, childMember4)
      & SERGUT_NESTED_MMEMBER(data, intMember5, nestedIntMember5)
      & SERGUT_MMEMBER(data, intMember6)
      & SERGUT_MMEMBER(data, stringMember7)
      & SERGUT_MMEMBER(data, charPtrMember8)
      & SERGUT_MMEMBER(data, charMember9)
      & SERGUT_MMEMBER(data, childVectorMember10)
      & SERGUT_MMEMBER(data, intVectorMember11)
      & SERGUT_MMEMBER(data, childMember12);
}


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
    const TestParent tp{ 21, 99, 124, TestChild{ -27, -42, {4, 45}, -23, 3.14159, 2.718, -127 }, 65000, 255,
                         "\nstring\\escaped\"quoted\" <b>Daten</b>foo", "char* Daten", 'c', { {22}, {33}, {44} }, { 1, 2, 3, 4}, { -99 } };

    WHEN("The datastructure is serialized to JSON") {
      sergut::JsonSerializer ser;
      ser.serializeData("Dummy", tp);

      THEN("The result is the specified string") {
        const std::string req = "{\"intMember1\":21,\"intMember2\":99,\"intMember3\":124,"
                                "\"childMember4\":{\"intMember1\":-27,\"intMember2\":-42,"
                                "\"timeMember3\":\"4:45:00\",\"intMember4\":-23,\"doubleMember5\":3.14159,"
                                "\"floatMember6\":2.718,\"intMember7\":-127},\"intMember5\":{\"nestedIntMember5\":65000},"
                                "\"intMember6\":255,\"stringMember7\":\"\\nstring\\\\escaped\\\"quoted\\\" <b>Daten<\\/b>foo\","
                                "\"charPtrMember8\":\"char* Daten\",\"charMember9\":\"c\","
                                "\"childVectorMember10\":[{\"grandChildValue\":22},{\"grandChildValue\":33},{\"grandChildValue\":44}],"
                                "\"intVectorMember11\":[1,2,3,4],\"childMember12\":{\"grandChildValue\":-99}}";
        CHECK(ser.str() == req);
      }
    }
    WHEN("The datastructure is serialized to XML") {
      sergut::XmlSerializer ser;
      ser.serializeData("Dummy", tp);

      THEN("The result is the specified string") {
        const std::string req =
            "<Dummy intMember1=\"21\" intMember2=\"99\"><intMember3>124</intMember3>"
            "<childMember4 intMember1=\"-27\" intMember2=\"-42\" timeMember3=\"4:45:00\" intMember4=\"-23\" doubleMember5=\"3.14159\" "
              "floatMember6=\"2.718\">-127</childMember4>"
            "<intMember5><nestedIntMember5>65000</nestedIntMember5></intMember5>"
            "<intMember6>255</intMember6><stringMember7>\nstring\\escaped&quot;quoted&quot; &lt;b&gt;Daten&lt;/b&gt;foo</stringMember7>"
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


struct Simple {
public:
  bool operator==(const Simple& rhs) const {
    return int1    == rhs.int1
        && double2 == rhs.double2
        && time3   == rhs.time3
        && char4   == rhs.char4
        && uchar5  == rhs.uchar5
        && time6   == rhs.time6;
  }
public:
  int int1;
  double double2;
  Time time3;
  char char4;
  unsigned char uchar5;
  Time time6;
};

SERGUT_FUNCTION(Simple, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, int1)
      & SERGUT_MMEMBER(data, double2)
      & SERGUT_MMEMBER(data, time3)
      & sergut::children
      & SERGUT_NESTED_MMEMBER(data, char4, nestedChar4)
      & SERGUT_MMEMBER(data, uchar5)
      & SERGUT_NESTED_MMEMBER(data, time6, nestedTime6)
      ;
}

TEST_CASE("Serialize simple class", "[sergut]")
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

int main_disabled(int /*argc*/, char */*argv*/[])
{
  const TestParent tp{ 21, 99, 124, TestChild{ -27, -42, {4, 45}, -23, 3.14159, 2.718, -127 }, 65000, 255,
                       "\x0fstring\\escaped\"quoted\" <b>Daten</b>foo", "char* Daten", 'c', { {22}, {33}, {44} }, { 1, 2, 3, 4}, { -99 } };

  {
    sergut::JsonSerializer ser;
    ser.serializeData("Dummy", tp);
    std::cout << ser.str() << std::endl;
  }
  std::cout << "\n\n";
  {



    const std::string res = [&]() {
      sergut::XmlSerializer ser;
      ser.serializeData("Dummy", tp);
      return ser.str();
    }();
    std::cout << "res: " << res << std::endl;
    sergut::XmlDeserializer deser{sergut::misc::ConstStringRef(res)};
    const TestParent tpDeser = deser.deserializeData<TestParent>("Dummy");
    const std::string res2 = [&]() {
      sergut::XmlSerializer ser;
      ser.serializeData("Dummy", tpDeser);
      return ser.str();
    }();
    std::cout << "res2: " << res2 << std::endl;
  }

  {
    const std::string origXml("<Dummy double2=\"2.345\" int1=\"12345\" time3=\"3:23:99\"><uchar5>21</uchar5><char4><nestedChar4>X</nestedChar4></char4><time6><nestedTime6>12:34:55</nestedTime6></time6></Dummy>");
    sergut::XmlDeserializer deser{sergut::misc::ConstStringRef(origXml)};
    const Simple res = deser.deserializeData<Simple>("Dummy");
    sergut::XmlSerializer ser;
    ser.serializeData("Dummy", res);
    std::cout << "Orig:  " << origXml << "\n"
              << "Later: " << ser.str() << "\n";
  }

//  // unsupported
//  {
//    const std::string origXml("<vector>1</vector><vector>2</vector>");
//    sergut::XmlDeserializer deser(origXml);
//    const std::vector<int> res = deser.deserializeData<std::vector<int>>("vector", sergut::XmlValueType::Child);
//    sergut::XmlSerializer ser;
//    ser.serializeData("vector", res);
//    std::cout << "Orig:  " << origXml << "\n"
//              << "Later: " << ser.str() << "\n";
//  }

  // unsupported
//  {
//    const std::string origXml("<vector>1:00:03</vector><vector>20:34:35</vector>");
//    sergut::XmlDeserializer deser(origXml);
//    const std::vector<Time> res =
//        deser.deserializeData<std::vector<Time>>("vector", sergut::XmlValueType::Child);
//    sergut::XmlSerializer ser;
//    ser.serializeData<std::vector<Time>>("vector", res);
//    std::cout << "Orig:  " << origXml << "\n"
//              << "Later: " << ser.str() << "\n";
//  }

//  {
//    sergut::XmlDeserializer deser(utf16Xml);
//    const TestParent tpDeser = deser.deserializeData<TestParent>("Dummy");
//    const std::string res2 = [&]() {
//      sergut::XmlSerializer ser;
//      ser.serializeData("Dummy", tpDeser);
//      return ser.str();
//    }();
//    std::cout << "res2: " << res2 << std::endl;
//  }
  return 0;
}
