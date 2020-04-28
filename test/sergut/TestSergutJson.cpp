#include <catch.hpp>

#include "TestSupportClasses.h"

#include "sergut/JsonSerializer.h"
#include "sergut/JsonDeserializer.h"

#include <rapidjson/document.h>


#include <string>
#include <vector>

TEST_CASE("RapidJSON", "[sergut]")
{
  GIVEN("A simple C++ datastructure")  {
    WHEN("The datastructure is fed to a rapidJSON-Document") {
      THEN("The result is the specified string") {
        rapidjson::Document doc;
        const char* data = "123";
        doc.Parse(data);
        CHECK(doc.GetInt() == 123);
      }
    }
  }
}

template<typename DT>
void testSerializeSimpleTypes(const DT& dt, const std::string& expectedResult) {
  GIVEN("A simple C++ datastructure: " + std::string(typeid(DT).name()))  {
    WHEN("The datastructure is serialized to JSON") {
      THEN("The result is the specified string") {
        sergut::JsonSerializer ser;
        ser.serializeData(dt);
        CHECK(ser.str() == expectedResult);
      }
    }
  }
}

TEST_CASE("Serialize simple types to JSON", "[sergut]")
{
  testSerializeSimpleTypes("abc", "\"abc\"");
  testSerializeSimpleTypes(9943, "9943");
  testSerializeSimpleTypes("Ich kann\nauch\x01mit Umlauten: \xc3\xa4 und Anf\xc3\xbchrungszeichen: \"", "\"Ich kann\\nauch\\u0001mit Umlauten: \xc3\xa4 und Anf\xc3\xbchrungszeichen: \\\"\"");
}



TEST_CASE("Serialize complex class to JSON", "[sergut]")
{

  GIVEN("A complex C++ POD datastructure")  {
    const TestParent tp{ 21, 99, 124, TestChild{ -27, -42, Time{4, 45}, -23, 3.14159, 2.718f, -127 }, 65000, 255,
                         "\nstring\\escaped\"quoted\" &<b>Daten</b>foo", "char* Daten", 'c', { {22}, {33}, {44} }, { 1, 2, 3, 4}, { -99 } };
    const std::string req = "{\"intMember1\":21,\"intMember2\":99,\"intMember3\":124,"
                            "\"childMember4\":{\"intMember1\":-27,\"intMember2\":-42,"
                            "\"timeMember3\":\"4:45:00\",\"intMember4\":-23,\"doubleMember5\":3.14159,"
                            "\"floatMember6\":2.718,\"intMember7\":-127},\"intMember5\":{\"nestedIntMember5\":65000},"
                            "\"intMember6\":255,\"stringMember7\":\"\\nstring\\\\escaped\\\"quoted\\\" &<b>Daten</b>foo\","
                            "\"charPtrMember8\":\"char* Daten\",\"charMember9\":\"c\","
                            "\"childVectorMember10\":[{\"grandChildValue\":22},{\"grandChildValue\":33},{\"grandChildValue\":44}],"
                            "\"intVectorMember11\":[1,2,3,4],\"childMember12\":{\"grandChildValue\":-99}}";

    WHEN("The datastructure is serialized to JSON") {
      sergut::JsonSerializer ser;
      ser.serializeData(tp);

      THEN("The result is the specified string") {
        CHECK(ser.str() == req);
      }
    }
    WHEN("The datastructure is deserialized from JSON") {
      sergut::JsonDeserializer ser(req);
      TestParent tst = ser.deserializeData<TestParent>();

      THEN("The result is the specified string") {
        CHECK(tp == tst);
      }
    }
  }
}
