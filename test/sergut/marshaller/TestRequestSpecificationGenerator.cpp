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
#include "sergut/marshaller/TestSupportClassesMarshaller.h"
#include "sergut/marshaller/RequestSpecificationGenerator.h"

#include <sstream>
#include <iomanip>

TEST_CASE("Generate request specification of call", "[RequestSpecificationGenerator]")
{
  GIVEN("A RequestSpecificationGenerator that is initialized with MyInterface") {
    sergut::marshaller::RequestSpecificationGenerator rsg;
    MyInterface::initialize(rsg, nullptr);
    WHEN("The list of all requests is queried") {
      THEN("The correct information is shown") {
        CHECK(rsg.getRequests().size() == 4);
        {
          const std::string requestSpecification = rsg.generateDocumentation("constructFromComplexParams",
                                                                             "application/xml");
          CHECK(requestSpecification ==
                "Request: p2.time=&p2.someLetter=&p2.someUnsignedShortInt=&p2.moreTime=&p1.time=&p1.someLetter=&p1.someUnsignedShortInt=&p1.moreTime=\n"
                "Input: <xs:complextype name='input'>\n"
                "<xs:attribute name='time' type='string'/>\n"
                "<xs:attribute name='someLetter' type='char'/>\n"
                "<xs:attribute name='someUnsignedShortInt' type='ushort'/>\n"
                "<xs:attribute name='moreTime' type='string'/>\n"
                "</xs:complextype>\n"
                "Response: <xs:complextype name='returnType'>\n"
                "<xs:attribute name='time' type='string'/>\n"
                "<xs:attribute name='someLetter' type='char'/>\n"
                "<xs:attribute name='someUnsignedShortInt' type='ushort'/>\n"
                "<xs:attribute name='moreTime' type='string'/>\n"
                "</xs:complextype>\n");
        }
        {
          const std::string requestSpecification = rsg.generateDocumentation("constructSomeMoreComplexTestData",
                                                                             "application/xml");
          CHECK(requestSpecification ==
                "Request: someUnsignedShortInt=&someLetter=&second1=&minute1=&hour1=\n"
                "Input: <xs:element name='time2' type='string'/>\n"
                "Response: <xs:complextype name='returnType'>\n"
                "<xs:attribute name='time' type='string'/>\n"
                "<xs:attribute name='someLetter' type='char'/>\n"
                "<xs:attribute name='someUnsignedShortInt' type='ushort'/>\n"
                "<xs:attribute name='moreTime' type='string'/>\n"
                "</xs:complextype>\n");
        }
        {
          const std::string requestSpecification = rsg.generateDocumentation("empty",
                                                                             "application/xml");
          CHECK(requestSpecification ==
                "Request: \n"
                "Input: Response: <xs:element name='rt' type='uint'/>\n");
        }
        {
          const std::string requestSpecification = rsg.generateDocumentation("sumUpSomeData",
                                                                             "application/xml");
          CHECK(requestSpecification ==
                "Request: otherUInt=&someUInt=\n"
                "Input: <xs:element name='t' type='string'/>\n"
                "Response: <xs:element name='returnUInt32' type='uint'/>\n");
        }
      }
    }
  }
}
