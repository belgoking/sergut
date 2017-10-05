#include <catch.hpp>

#include "TestSupportClasses.h"

#include "sergut/JsonSerializer.h"
//#include "sergut/JsonDeserializer.h"

#include <string>
#include <vector>

TEST_CASE("Serialize complex class to JSON", "[sergut]")
{

  GIVEN("A complex C++ POD datastructure")  {
    const TestParent tp{ 21, 99, 124, TestChild{ -27, -42, {4, 45}, -23, 3.14159, 2.718, -127 }, 65000, 255,
                         "\nstring\\escaped\"quoted\" &<b>Daten</b>foo", "char* Daten", 'c', { {22}, {33}, {44} }, { 1, 2, 3, 4}, { -99 } };

    WHEN("The datastructure is serialized to JSON") {
      sergut::JsonSerializer ser;
      ser.serializeData("Dummy", tp);

      THEN("The result is the specified string") {
        const std::string req = "{\"intMember1\":21,\"intMember2\":99,\"intMember3\":124,"
                                "\"childMember4\":{\"intMember1\":-27,\"intMember2\":-42,"
                                "\"timeMember3\":\"4:45:00\",\"intMember4\":-23,\"doubleMember5\":3.14159,"
                                "\"floatMember6\":2.718,\"intMember7\":-127},\"intMember5\":{\"nestedIntMember5\":65000},"
                                "\"intMember6\":255,\"stringMember7\":\"\\nstring\\\\escaped\\\"quoted\\\" &<b>Daten<\\/b>foo\","
                                "\"charPtrMember8\":\"char* Daten\",\"charMember9\":\"c\","
                                "\"childVectorMember10\":[{\"grandChildValue\":22},{\"grandChildValue\":33},{\"grandChildValue\":44}],"
                                "\"intVectorMember11\":[1,2,3,4],\"childMember12\":{\"grandChildValue\":-99}}";
        CHECK(ser.str() == req);
      }
    }
  }
}
