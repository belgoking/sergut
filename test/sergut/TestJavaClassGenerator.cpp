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

#if 0

#include "sergut/JavaClassGenerator.h"
#include "sergut/detail/JavaClassGeneratorBuilder.h"

#include "TestSupportClasses.h"

#include <catch.hpp>

#include <sstream>





////////////////////////////////////////////////////////////////////////////////
// Test basic datatypes
////////////////////////////////////////////////////////////////////////////////
struct AllBasicTypes {
  bool _bool;
  char _char;
  signed char _signed_char;
  unsigned char _unsigned_char;
  signed short _signed_short;
  unsigned short _unsigned_short;
  signed int _signed_int;
  unsigned int _unsigned_int;
  signed long _signed_long;
  unsigned long _unsigned_long;
  signed long long _signed_long_long;
  unsigned long long _unsigned_long_long;
  float _float;
  double _double;
  const char* _char_ptr;
  std::string _str;
};
SERGUT_FUNCTION(AllBasicTypes, data, ar) {
  ar
      & SERGUT_MMEMBER(data, _bool)
      & SERGUT_MMEMBER(data, _char)
      & SERGUT_MMEMBER(data, _signed_short)
      & SERGUT_MMEMBER(data, _unsigned_short)
      & SERGUT_MMEMBER(data, _signed_int)
      & SERGUT_MMEMBER(data, _unsigned_int)
      & SERGUT_MMEMBER(data, _signed_long)
      & SERGUT_MMEMBER(data, _unsigned_long)
      & SERGUT_MMEMBER(data, _signed_long_long)
      & SERGUT_MMEMBER(data, _unsigned_long_long)
      & SERGUT_MMEMBER(data, _float)
      & SERGUT_MMEMBER(data, _double)
      & SERGUT_MMEMBER(data, _char_ptr)
      & SERGUT_MMEMBER(data, _str);
}

TEST_CASE("Generate Java Class with all basic types", "[JavaGen]")
{
  SECTION("Simple Class") {
    const sergut::JavaClassGenerator cls = sergut::JavaClassGenerator::generate<AllBasicTypes>();
    std::ostringstream ostr;
    ostr << cls;
    CHECK(ostr.str() == std::string());
  }
}

////////////////////////////////////////////////////////////////////////////////
// Check for correct sizes of basic datatypes
////////////////////////////////////////////////////////////////////////////////
struct AllBasicTypeSizes {
  bool _bool;
  char _char;
  int8_t _int8;
  uint8_t _uint8;
  int16_t _int16;
  uint16_t _uint16;
  int32_t _int32;
  uint32_t _uint32;
  int64_t _int64;
  uint64_t _uint64;
  float _float;
  double _double;
  const char* _char_ptr;
  std::string _str;
};
SERGUT_FUNCTION(AllBasicTypeSizes, data, ar) {
  ar
      & SERGUT_MMEMBER(data, _bool)
      & SERGUT_MMEMBER(data, _char)
      & SERGUT_MMEMBER(data, _int8)
      & SERGUT_MMEMBER(data, _uint8)
      & SERGUT_MMEMBER(data, _int16)
      & SERGUT_MMEMBER(data, _uint16)
      & SERGUT_MMEMBER(data, _int32)
      & SERGUT_MMEMBER(data, _uint32)
      & SERGUT_MMEMBER(data, _int64)
      & SERGUT_MMEMBER(data, _uint64)
      & SERGUT_MMEMBER(data, _float)
      & SERGUT_MMEMBER(data, _double)
      & SERGUT_MMEMBER(data, _char_ptr)
      & SERGUT_MMEMBER(data, _str);
}

TEST_CASE("Generate Java Class and check basic type sizes", "[JavaGen]")
{
  SECTION("Simple Class") {
    const sergut::JavaClassGenerator cls = sergut::JavaClassGenerator::generate<AllBasicTypeSizes>();
    std::ostringstream ostr;
    ostr << cls;
    CHECK(ostr.str() == std::string());
  }
}



////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
struct C1 {
  std::uint8_t uint8_1;
};
SERGUT_FUNCTION(C1, data, ar) {
  ar
      & SERGUT_MMEMBER(data, uint8_1);
}

namespace ns1 {
namespace ns2 {
struct C1 {
  std::uint8_t uint8_1;
};
SERGUT_FUNCTION(ns1::ns2::C1, data, ar) {
  ar
      & SERGUT_MMEMBER(data, uint8_1);
}

namespace ns3 {
struct C1 {
  std::uint8_t uint8_1;
};
SERGUT_FUNCTION(ns1::ns2::ns3::C1, data, ar) {
  ar
      & SERGUT_MMEMBER(data, uint8_1);
}

struct C1231 {
  std::string str;
  C1 c1;
};
SERGUT_FUNCTION(ns1::ns2::ns3::C1231, data, ar) {
  ar
      & SERGUT_MMEMBER(data, str)
      & sergut::children
      & SERGUT_MMEMBER(data, c1);
}

struct C1232 {
  std::string str;
  C1231 c1231;
};
SERGUT_FUNCTION(ns1::ns2::ns3::C1232, data, ar) {
  ar
      & SERGUT_MMEMBER(data, str)
      & sergut::children
      & SERGUT_MMEMBER(data, c1231);
}

struct C1233 {
  ns1::ns2::ns3::C1 ns1_ns2_ns3_C1;
  ns1::ns2::C1 ns1_ns2_C1;
  C1 globalNs_C1;
};
SERGUT_FUNCTION(ns1::ns2::ns3::C1233, data, ar) {
  ar
      & SERGUT_MMEMBER(data, ns1_ns2_ns3_C1)
      & SERGUT_MMEMBER(data, ns1_ns2_C1)
      & sergut::children
      & SERGUT_MMEMBER(data, globalNs_C1);
}

}
struct C121 {
  ns3::C1231 c1231_1;
  ns3::C1232 c1232;
  ns3::C1231 c1231_2;
};
SERGUT_FUNCTION(ns1::ns2::C121, data, ar) {
  ar
      & SERGUT_MMEMBER(data, c1231_1)
      & SERGUT_MMEMBER(data, c1232)
      & sergut::children
      & SERGUT_MMEMBER(data, c1231_2);
}

}
}

struct C02 {
  ns1::ns2::ns3::C1232 c1232;
};
SERGUT_FUNCTION(C02, data, ar) {
  ar
      & SERGUT_MMEMBER(data, c1232);
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
TEST_CASE("Generate Java Class", "[JavaGen]")
{
  SECTION("Simple Class") {
    const sergut::JavaClassGenerator cls = sergut::JavaClassGenerator::generate<SomeTestData>();
    CHECK(cls.getPath() == std::string("SomeTestData.java"));
    std::ostringstream ostr;
    ostr << cls;
    CHECK(ostr.str() == std::string(""));
  }
  SECTION("More complex Class") {
    const sergut::JavaClassGenerator cls = sergut::JavaClassGenerator::generate<SomeMoreComplexTestData>();
    CHECK(cls.getPath() == "SomeMoreComplexTestData.java");
    std::ostringstream ostr;
    ostr << cls;
    CHECK(ostr.str() == std::string(""));
  }
}

TEST_CASE("Generate Java Class with NameSpaces", "[JavaGen]")
{
  SECTION("With Package and Members without Package") {
    const sergut::JavaClassGenerator cls = sergut::JavaClassGenerator::generate<ns1::ns2::ns3::C1231>();
    CHECK(cls.getPath() == "ns1/ns2/ns3/C1231.java");
    std::ostringstream ostr;
    ostr << cls;
    CHECK(ostr.str() == std::string(""));
  }
  SECTION("With Package and Members from same Package") {
    const sergut::JavaClassGenerator cls = sergut::JavaClassGenerator::generate<ns1::ns2::ns3::C1232>();
    CHECK(cls.getPath() == "ns1/ns2/ns3/C1232.java");
    std::ostringstream ostr;
    ostr << cls;
    CHECK(ostr.str() == std::string(""));
  }
  SECTION("With Package and Members from different Packages") {
    const sergut::JavaClassGenerator cls = sergut::JavaClassGenerator::generate<ns1::ns2::ns3::C1233>();
    CHECK(cls.getPath() == "ns1/ns2/ns3/C1233.java");
    std::ostringstream ostr;
    ostr << cls;
    CHECK(ostr.str() == std::string(""));
  }
}


////////////////////////////////////////////////////////////////////////////////
// Tests with collections
/////////////////////////////////////////////////////////////////////////////////
struct ArrayList {
  int32_t someInt32;
};
SERGUT_FUNCTION(ArrayList, data, ar) {
  ar & SERGUT_MMEMBER(data, someInt32);
}
namespace myNameSpace {
struct ArrayList {
  int32_t someInt32;
};
SERGUT_FUNCTION(myNameSpace::ArrayList, data, ar) {
  ar & SERGUT_MMEMBER(data, someInt32);
}

struct TestClassWithCollection {
  std::vector<ns1::ns2::C1> arrayOf__ns1_ns2_C1;

};
SERGUT_FUNCTION(myNameSpace::TestClassWithCollection, data, ar) {
  ar & SERGUT_MMEMBER(data, arrayOf__ns1_ns2_C1);
}

struct TestClassWithCollectionNameSpaceClash {
  std::vector<ns1::ns2::C1> arrayOf__ns1_ns2_C1;
  myNameSpace::ArrayList memberOfClass__myNameSpace_ArrayList;

};
SERGUT_FUNCTION(myNameSpace::TestClassWithCollectionNameSpaceClash, data, ar) {
  ar
      & SERGUT_MMEMBER(data, arrayOf__ns1_ns2_C1)
      & SERGUT_MMEMBER(data, memberOfClass__myNameSpace_ArrayList);
}

struct TestClassWithCollectionNameSpaceClashGlobal {
  std::vector<ns1::ns2::C1> arrayOf__ns1_ns2_C1;
  ::ArrayList memberOfClass__ArrayList;
};
SERGUT_FUNCTION(myNameSpace::TestClassWithCollectionNameSpaceClashGlobal, data, ar) {
  ar
      & SERGUT_MMEMBER(data, arrayOf__ns1_ns2_C1)
      & SERGUT_MMEMBER(data, memberOfClass__ArrayList);
}

struct TestClassWithCollectionNameSpaceClashAndVectorElementNameSpaceClash {
  std::vector<ns1::ns2::C1> arrayOf__ns1_ns2_C1;
  ns1::ns2::ns3::C1 ns1_ns2_ns3_C1;
  myNameSpace::ArrayList memberOfClass__myNameSpace_ArrayList;

};
SERGUT_FUNCTION(myNameSpace::TestClassWithCollectionNameSpaceClashAndVectorElementNameSpaceClash, data, ar) {
  ar
      & SERGUT_MMEMBER(data, arrayOf__ns1_ns2_C1)
      & SERGUT_MMEMBER(data, ns1_ns2_ns3_C1)
      & SERGUT_MMEMBER(data, memberOfClass__myNameSpace_ArrayList);
}

struct TestClassWithCollectionNameSpaceClash2AndVectorElementNameSpaceClash {
  std::vector<ns1::ns2::C1> arrayOf__ns1_ns2_C1;
  ns1::ns2::ns3::C1 ns1_ns2_ns3_C1;
  myNameSpace::ArrayList memberOfClass__myNameSpace_ArrayList;
  ::ArrayList memberOfClass__ArrayList;

};
SERGUT_FUNCTION(myNameSpace::TestClassWithCollectionNameSpaceClash2AndVectorElementNameSpaceClash, data, ar) {
  ar
      & SERGUT_MMEMBER(data, arrayOf__ns1_ns2_C1)
      & SERGUT_MMEMBER(data, ns1_ns2_ns3_C1)
      & SERGUT_MMEMBER(data, memberOfClass__myNameSpace_ArrayList)
      & SERGUT_MMEMBER(data, memberOfClass__ArrayList);
}

}

TEST_CASE("Generate Java Class with collection", "[JavaGen]")
{
  SECTION("Simple Class with vector") {
    std::ostringstream ostr;
    ostr << sergut::JavaClassGenerator::generate<myNameSpace::TestClassWithCollection>();
    CHECK(ostr.str() == std::string(""));
  }
  SECTION("Class with vector and vector namespace clash") {
    std::ostringstream ostr;
    ostr << sergut::JavaClassGenerator::generate<myNameSpace::TestClassWithCollectionNameSpaceClash>();
    CHECK(ostr.str() == std::string(""));
  }
  SECTION("Class with vector and vector namespace clash from global namespace") {
    std::ostringstream ostr;
    ostr << sergut::JavaClassGenerator::generate<myNameSpace::TestClassWithCollectionNameSpaceClashGlobal>();
    CHECK(ostr.str() == std::string(""));
  }
  SECTION("Class with vector and vector namespace clash and vector element namespace clash") {
    std::ostringstream ostr;
    ostr << sergut::JavaClassGenerator::generate<myNameSpace::TestClassWithCollectionNameSpaceClashAndVectorElementNameSpaceClash>();
    CHECK(ostr.str() == std::string(""));
  }
  SECTION("Class with vector and two vector namespace clashes and vector element namespace clash") {
    std::ostringstream ostr;
    ostr << sergut::JavaClassGenerator::generate<myNameSpace::TestClassWithCollectionNameSpaceClash2AndVectorElementNameSpaceClash>();
    CHECK(ostr.str() == std::string(""));
  }
}

////////////////////////////////////////////////////////////////////////////////
// Tests with type mapping
/////////////////////////////////////////////////////////////////////////////////
struct TestClassWithTime {
  int8_t int8_1;
  Time time;
  uint8_t uint8_2;
};
SERGUT_FUNCTION(TestClassWithTime, data, ar) {
  ar
      & SERGUT_MMEMBER(data, int8_1)
      & SERGUT_MMEMBER(data, time)
      & SERGUT_MMEMBER(data, uint8_2);
}

class MyJavaClassGenerator: public sergut::detail::JavaClassGeneratorBuilder<MyJavaClassGenerator> {
public:
  sergut::detail::TypeName getTypeMapping(const std::string& cppTypeName,
                                          const sergut::detail::TypeName::CollectionType collectionType) const override
  {
    if(cppTypeName == "Time") {
      return sergut::detail::TypeName("java::lang::DateTime", collectionType);
    }
    return sergut::detail::JavaClassGeneratorBase::getTypeMapping(cppTypeName, collectionType);
  }
};

TEST_CASE("Generate Java Class overriding a datatype", "[JavaGen]")
{
  SECTION("Override Time") {
    std::ostringstream ostr;
    ostr << MyJavaClassGenerator::generate<TestClassWithTime>();
    CHECK(ostr.str() == std::string(""));
  }
}

#endif
