#include <catch.hpp>

#include "TestSupportClasses.h"

#include "sergut/UrlDeserializer.h"
#include "sergut/UrlSerializeToVector.h"
#include "sergut/UrlSerializer.h"

TEST_CASE("Serialize complex class to URL", "[sergut]")
{

  GIVEN("A complex C++ POD datastructure")  {
    const TestParent tp{ 21, 99, 124, TestChild{ -27, -42, Time{4, 45}, -23, 3.14159, 2.718, -127 }, 65000, 255,
                         "\nstring\\escaped\"quoted\" &<b>Daten</b>foo", "char* Daten", 'c', { {22}, {33}, {44} }, { 1, 2, 3, 4}, { -99 } };

    WHEN("The datastructure is serialized to URL (vector of pairs)") {
      sergut::UrlSerializeToVector ser;
      ser.serializeData("outer", tp);

      THEN("The result is the specified string") {
        const std::vector<std::pair<std::string,std::string>> req =
            {{"outer.intMember1", "21"}, {"outer.intMember2", "99"}, {"outer.intMember3", "124"},
             {"outer.childMember4.intMember1", "-27"}, {"outer.childMember4.intMember2", "-42"},
             {"outer.childMember4.timeMember3", "4:45:00"}, {"outer.childMember4.intMember4", "-23"},
             {"outer.childMember4.doubleMember5", "3.14159"}, {"outer.childMember4.floatMember6", "2.718"},
             {"outer.childMember4.intMember7", "-127"}, {"outer.intMember5.nestedIntMember5", "65000"},
             {"outer.intMember6", "255"}, {"outer.stringMember7", "\nstring\\escaped\"quoted\" &<b>Daten</b>foo"},
             {"outer.charPtrMember8", "char* Daten"}, {"outer.charMember9", "c"},
             {"outer.childVectorMember10.grandChildValue", "22"}, {"outer.childVectorMember10.grandChildValue", "33"},
             {"outer.childVectorMember10.grandChildValue", "44"}, {"outer.intVectorMember11", "1"},
             {"outer.intVectorMember11", "2"}, {"outer.intVectorMember11", "3"}, {"outer.intVectorMember11", "4"},
             {"outer.childMember12.grandChildValue", "-99"}};
        CHECK(ser.getParams() == req);
      }
    }

    WHEN("The datastructure is serialized to URL") {
      sergut::UrlSerializer ser;
      ser.serializeData("outer", tp);

      THEN("The result is the specified string") {
        const std::string req = "outer.intMember1=21&outer.intMember2=99&outer.intMember3=124&"
            "outer.childMember4.intMember1=-27&outer.childMember4.intMember2=-42&outer.childMember4.timeMember3=4%3a45%3a00&"
            "outer.childMember4.intMember4=-23&outer.childMember4.doubleMember5=3.14159&"
            "outer.childMember4.floatMember6=2.718&outer.childMember4.intMember7=-127&"
            "outer.intMember5.nestedIntMember5=65000&"
            "outer.intMember6=255&outer.stringMember7=%0astring%5cescaped%22quoted%22+%26%3cb%3eDaten%3c%2fb%3efoo&"
            "outer.charPtrMember8=char%2a+Daten&outer.charMember9=c&outer.childVectorMember10.grandChildValue=22&"
            "outer.childVectorMember10.grandChildValue=33&outer.childVectorMember10.grandChildValue=44&"
            "outer.intVectorMember11=1&outer.intVectorMember11=2&outer.intVectorMember11=3&"
            "outer.intVectorMember11=4&outer.childMember12.grandChildValue=-99";
        CHECK(ser.str() == req);
      }
    }
  }
}

namespace {
struct TestParentUrl
{
public:
  bool operator==(const TestParentUrl& rhs) const {
    return
        intMember1          == rhs.intMember1          &&
        intMember2          == rhs.intMember2          &&
        intMember3          == rhs.intMember3          &&
        childMember4        == rhs.childMember4        &&
        intMember5          == rhs.intMember5          &&
        intMember6          == rhs.intMember6          &&
        stringMember7       == rhs.stringMember7       &&
        charPtrMember8      == rhs.charPtrMember8      &&
        charMember9         == rhs.charMember9         &&
        intVectorMember10   == rhs.intVectorMember10   &&
        childMember11       == rhs.childMember11;
  }
public:
  unsigned long long intMember1;
  unsigned long intMember2;
  unsigned int intMember3;
  TestChild childMember4;
  unsigned short intMember5;
  unsigned char intMember6;
  std::string stringMember7;
  std::string charPtrMember8;
  char charMember9;
  std::vector<int> intVectorMember10;
  TestChild2 childMember11;
};

SERGUT_FUNCTION(TestParentUrl, data, ar)
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
      & SERGUT_MMEMBER(data, intVectorMember10)
      & SERGUT_MMEMBER(data, childMember11);
}
}


TEST_CASE("Deserialize an URL into class", "[sergut]")
{
  GIVEN("An URL request split into a vector of pairs and a simple class with members in wrong Order") {
    const std::vector<std::pair<std::string,std::string>> origRequest{
      {"outer.double2", "2.345"}, {"outer.int1", "12345"},          {"outer.time3", "3:23:99"},
      {"outer.uchar5", "21"},     {"outer.char4.nestedChar4", "X"}, {"outer.time6.nestedTime6", "12:34:55"}
    };
    const Simple origVal{ 12345, 2.345, Time{3, 23, 99}, 'X', 21, Time{12, 34, 55}};
    WHEN("The URL-Parameters are deserialized into the simple class (UrlDeserializer)") {
      sergut::UrlDeserializer deser{origRequest};
      const Simple res = deser.deserializeData<Simple>("outer");
      CHECK(res == origVal);
    }
  }

  GIVEN("An URL request split into a vector of pairs and a complex class with members in wrong Order") {
    const std::vector<std::pair<std::string,std::string>> origRequest{
      {"outer.intMember1", "21"}, {"outer.intMember2", "99"}, {"outer.intMember3", "124"},
      {"outer.childMember4.intMember1", "-27"}, {"outer.childMember4.intMember2", "-42"},
      {"outer.childMember4.timeMember3", "4:45:00"}, {"outer.childMember4.intMember4", "-23"},
      {"outer.childMember4.doubleMember5", "3.14159"}, {"outer.childMember4.floatMember6", "2.718"},
      {"outer.childMember4.intMember7", "-127"}, {"outer.intMember5.nestedIntMember5", "65000"}, {"outer.intMember6", "255"},
      {"outer.stringMember7", "\nstring\\escaped\"quoted\" &<b>Daten</b>foo"}, {"outer.charPtrMember8", "char* Daten"},
      {"outer.charMember9", "c"},
      {"outer.intVectorMember10", "1"}, {"outer.intVectorMember10", "2"}, {"outer.intVectorMember10", "3"},
      {"outer.intVectorMember10", "4"}, {"outer.childMember11.grandChildValue", "-99"}
      };
    const TestParentUrl origVal{ 21, 99, 124, TestChild{ -27, -42, Time{4, 45}, -23, 3.14159, 2.718, -127 }, 65000, 255,
                                 "\nstring\\escaped\"quoted\" &<b>Daten</b>foo", "char* Daten", 'c', { 1, 2, 3, 4}, { -99 } };

    WHEN("The URL-Parameters are deserialized into the simple class (UrlDeserializer)") {
      sergut::UrlDeserializer deser{origRequest};
      const TestParentUrl res = deser.deserializeData<TestParentUrl>("outer");
      CHECK(res == origVal);
    }
  }
}

TEST_CASE("Serialize/Deserialize several classes to Url", "[sergut]")
{
  const TestParentUrl tp1{ 21, 99, 124, TestChild{ -27, -42, Time{4, 45}, -23, 3.14159, 2.718, -127 }, 65000, 255,
                       "\nstring\\escaped\"quoted\" &<b>Daten</b>foo", "char* Daten", 'c', { 1, 2, 3, 4}, { -99 } };
  const Simple simple2{ 12345, 2.345, Time{3, 23, 99}, 'X', 21, Time{12, 34, 55}};
  const Simple simple3{ 54321, 543.2, Time{23, 3, 01}, 'Y', 12, Time{21, 55, 34}};

  GIVEN("Three classes") {
    WHEN("The datastructure is serialized to URL") {
      sergut::UrlSerializer ser;
      ser.serializeData("tp1",     tp1);
      ser.serializeData("simple2", simple2);
      ser.serializeData("simple3", simple3);
      THEN("The result is the specified string") {
        const std::string req = "tp1.intMember1=21&tp1.intMember2=99&tp1.intMember3=124&"
            "tp1.childMember4.intMember1=-27&tp1.childMember4.intMember2=-42&tp1.childMember4.timeMember3=4%3a45%3a00&"
            "tp1.childMember4.intMember4=-23&tp1.childMember4.doubleMember5=3.14159&"
            "tp1.childMember4.floatMember6=2.718&tp1.childMember4.intMember7=-127&"
            "tp1.intMember5.nestedIntMember5=65000&"
            "tp1.intMember6=255&tp1.stringMember7=%0astring%5cescaped%22quoted%22+%26%3cb%3eDaten%3c%2fb%3efoo&"
            "tp1.charPtrMember8=char%2a+Daten&tp1.charMember9=c&"
            "tp1.intVectorMember10=1&tp1.intVectorMember10=2&tp1.intVectorMember10=3&"
            "tp1.intVectorMember10=4&tp1.childMember11.grandChildValue=-99&"

            "simple2.int1=12345&simple2.double2=2.345&simple2.time3=3%3a23%3a99&"
            "simple2.char4.nestedChar4=X&simple2.uchar5=21&simple2.time6.nestedTime6=12%3a34%3a55&"

            "simple3.int1=54321&simple3.double2=543.2&simple3.time3=23%3a03%3a01&"
            "simple3.char4.nestedChar4=Y&simple3.uchar5=12&simple3.time6.nestedTime6=21%3a55%3a34";
        CHECK(ser.str() == req);
      }
    }
  }

  GIVEN("An URL request split into a multimap and a complex class with members in wrong Order") {
    const std::string origXml();
    const std::vector<std::pair<std::string,std::string>> origRequest{
      {"simple2.double2", "2.345"}, {"simple2.int1", "12345"},          {"simple2.time3", "3:23:99"},
      {"simple2.uchar5", "21"},     {"simple2.char4.nestedChar4", "X"}, {"simple2.time6.nestedTime6", "12:34:55"},
      {"tp1.intMember1", "21"}, {"tp1.intMember2", "99"}, {"tp1.intMember3", "124"},
      {"tp1.childMember4.intMember1", "-27"}, {"tp1.childMember4.intMember2", "-42"},
      {"tp1.childMember4.timeMember3", "4:45:00"}, {"tp1.childMember4.intMember4", "-23"},
      {"tp1.childMember4.doubleMember5", "3.14159"}, {"tp1.childMember4.floatMember6", "2.718"},
      {"tp1.childMember4.intMember7", "-127"}, {"tp1.intMember5.nestedIntMember5", "65000"}, {"tp1.intMember6", "255"},
      {"tp1.stringMember7", "\nstring\\escaped\"quoted\" &<b>Daten</b>foo"}, {"tp1.charPtrMember8", "char* Daten"},
      {"tp1.charMember9", "c"},
      {"tp1.intVectorMember10", "1"}, {"tp1.intVectorMember10", "2"}, {"tp1.intVectorMember10", "3"},
      {"tp1.intVectorMember10", "4"}, {"tp1.childMember11.grandChildValue", "-99"},
      {"simple3.double2", "543.2"}, {"simple3.int1", "54321"},          {"simple3.time3", "23:03:01"},
      {"simple3.uchar5", "12"},     {"simple3.char4.nestedChar4", "Y"}, {"simple3.time6.nestedTime6", "21:55:34"}
      };
    WHEN("The URL-Parameters are deserialized into the classes (UrlDeserializer)") {
      sergut::UrlDeserializer deser{origRequest};
      const TestParentUrl resTp1 = deser.deserializeData<TestParentUrl>("tp1");
      CHECK(resTp1 == tp1);
      const Simple resSimple2 = deser.deserializeData<Simple>("simple2");
      CHECK(resSimple2 == simple2);
      const Simple resSimple3 = deser.deserializeData<Simple>("simple3");
      CHECK(resSimple3 == simple3);
    }
  }
}
