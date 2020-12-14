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

#include "sergut/TestSupportClasses.h"
#include "sergut/XsdGenerator.h"

#include <sstream>
#include <iomanip>

TEST_CASE("Generate XML specification for class", "[XsdGenerator]")
{
  GIVEN("A XsdGenerator for a simple class") {
    std::ostringstream ostr;
    sergut::XsdGenerator ser(ostr);
    WHEN("A Xsd is generated") {
      THEN("The correct information is shown") {
        ser.serializeData<SomeMoreComplexTestData>("input");
        CHECK(ostr.str() ==
              "<xs:complextype name='input'>\n"
              "<xs:attribute name='time' type='string'/>\n"
              "<xs:attribute name='someLetter' type='char'/>\n"
              "<xs:attribute name='someUnsignedShortInt' type='ushort'/>\n"
              "<xs:attribute name='moreTime' type='string'/>\n"
              "</xs:complextype>\n");
      }
    }
  }
  GIVEN("A XsdGenerator for a class with single child") {
    std::ostringstream ostr;
    sergut::XsdGenerator ser(ostr);
    WHEN("A Xsd is generated") {
      THEN("The correct information is shown") {
        ser.serializeData<SingleChildTestData>("input");
        CHECK(ostr.str() ==
              "<xs:complextype name='input' type='uint'>\n"
              "<xs:attribute name='attributeMember' type='uint'/>\n"
              "</xs:complextype>\n");
      }
    }
  }
}
