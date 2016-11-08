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

#include "sergut/TestSupportClasses.h"
#include "sergut/marshaller/TestSupportClassesMarshaller.h"

#include <sstream>
#include <iomanip>


TEST_CASE("Call simple function 1 with RequestServer", "[RequestServer]")
{
  GIVEN("A RequestServer") {
    MyInterfaceServer myInterfaceServer;
    WHEN("A request comes in") {
      RequestMock request{ "sumUpSomeData", {{"someUInt", "3"}, {"otherUInt", "5"}}, "<t>23:12:20</t>" };
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myInterfaceServer.call(request) == "<returnUInt32>231228</returnUInt32>");
      }
    }
  }
}

TEST_CASE("Call more complex function 2 with RequestServer", "[RequestServer]")
{
  GIVEN("A RequestServer") {
    MyInterfaceServer myInterfaceServer;
    WHEN("A request comes in") {
      RequestMock request{ "constructSomeMoreComplexTestData", {
          {"someLetter", "b"}, {"second1", "1"}, {"hour1", "2"}, {"minute1", "3"},
                       {"someUnsignedShortInt", "123"}}, "<time2>23:12:20</time2>" };
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myInterfaceServer.call(request) ==
              "<returnType time=\"2:03:01\" someLetter=\"b\" someUnsignedShortInt=\"123\" moreTime=\"23:12:20\"/>");
      }
    }
  }
}

TEST_CASE("Call more complex function 3 with RequestServer", "[RequestServer]")
{
  GIVEN("A RequestServer") {
    MyInterfaceServer myInterfaceServer;
    WHEN("A request comes in") {
      RequestMock request{ "constructFromComplexParams", {
          {"p1.time", "3:03:03"}, {"p1.someLetter", "c"}, {"p1.someUnsignedShortInt", "3"}, {"p1.moreTime", "6:06:06"},
          {"p2.time", "4:04:04"}, {"p2.someLetter", "d"}, {"p2.someUnsignedShortInt", "4"}, {"p2.moreTime", "7:07:07"}},
          "<input time=\"1:02:03\" someLetter=\"b\" someUnsignedShortInt=\"123\" moreTime=\"23:12:20\"/>" };
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myInterfaceServer.call(request) ==
              "<returnType time=\"1:02:03\" someLetter=\"c\" someUnsignedShortInt=\"7\" moreTime=\"7:07:07\"/>");
      }
    }
  }
}
