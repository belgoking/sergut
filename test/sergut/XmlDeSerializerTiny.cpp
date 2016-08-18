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

#include "sergut/Util.h"
#include "sergut/JsonSerializer.h"
#include "sergut/XmlDeserializerTiny.h"
#include "sergut/XmlSerializer.h"
#include "sergut/SerializerBase.h"
#include "sergut/DeserializerBase.h"

#include <cctype>
#include <cinttypes>
#include <iomanip>
#include <iostream>
#include <vector>

struct SomeTestData {
  SomeTestData() = default;
  SomeTestData(const uint32_t m, const uint32_t o) : mandatoryMember(m), optionalMember(o) { }
  bool operator==(const SomeTestData& rhs) const
  {
    return mandatoryMember == rhs.mandatoryMember &&
        optionalMember == rhs.optionalMember;
  }
  uint32_t mandatoryMember = 0;
  uint32_t optionalMember = 0;
};
SERGUT_FUNCTION(SomeTestData, data, ar) {
  ar & sergut::children
      & SERGUT_MMEMBER(data, mandatoryMember)
      & SERGUT_OMEMBER(data, optionalMember);
}

// this is a prequel to the checks for invalid XML
TEST_CASE("Deserialize valid XML (tiny)", "[sergut]")
{
  SECTION("OK-Test") {
    sergut::XmlDeserializerTiny deser("<rootTag><mandatoryMember>10</mandatoryMember><optionalMember>23</optionalMember></rootTag>");
    const SomeTestData td = deser.deserializeData<SomeTestData>("rootTag");
    CHECK(td == (SomeTestData{10, 23}));
  }
  SECTION("OK-Test with missing optional") {
    sergut::XmlDeserializerTiny deser("<rootTag><mandatoryMember>10</mandatoryMember></rootTag>");
    const SomeTestData td = deser.deserializeData<SomeTestData>("rootTag");
    CHECK(td == (SomeTestData{10, 0}));
  }
  SECTION("OK-Test") {
    sergut::XmlDeserializerTiny deser("<rootTag><mandatoryMember>10</mandatoryMember><optionalMember>23</optionalMember></rootTag>");
    const SomeTestData td = deser.deserializeData<SomeTestData>(nullptr);
    CHECK(td == (SomeTestData{10, 23}));
  }
}

TEST_CASE("Deserialize invalid XML (tiny)", "[sergut]")
{
  SECTION("Invalid XML") {
    CHECK_THROWS_AS(sergut::XmlDeserializerTiny("<tag1><someMember>10</someMember></wrongTag1>"), sergut::ParsingException);
  }
  SECTION("Wrong root tag") {
    sergut::XmlDeserializerTiny deser("<wrongTag1><mandatoryMember>10</mandatoryMember></wrongTag1>");
    CHECK_THROWS_AS(deser.deserializeData<SomeTestData>("otherRootTag"), sergut::ParsingException);
  }
  SECTION("Mandatory member missing") {
    sergut::XmlDeserializerTiny deser("<rootTag><someWrongMember>10</someWrongMember></rootTag>");
    CHECK_THROWS_AS(deser.deserializeData<SomeTestData>("rootTag"), sergut::ParsingException);
  }
}
