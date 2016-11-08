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


TEST_CASE("Call simple function 1 with RequestClient", "[RequestClient]")
{
  GIVEN("A RequestServer") {
    RequestHandlerMock requestHandler;
    MyInterfaceClient myInterfaceClient(requestHandler);
    WHEN("A request comes in") {
      {
        const std::string res("<rt>42</rt>");
        requestHandler._response = std::make_pair("application/xml", std::vector<char>(res.begin(), res.end()));
      }
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myInterfaceClient.empty() == 42);
        CHECK(requestHandler._seenRequest._functionName == "empty");
        CHECK(requestHandler._seenRequest._input == "");
        CHECK(requestHandler._seenRequest._params ==
              (std::vector<std::pair<std::string,std::string>>{}));
      }
    }
  }
}

TEST_CASE("Call simple function 2 with RequestClient", "[RequestClient]")
{
  GIVEN("A RequestServer") {
    RequestHandlerMock requestHandler;
    MyInterfaceClient myInterfaceClient(requestHandler);
    WHEN("A request comes in") {
      {
        const std::string res("<returnUInt32>23</returnUInt32>");
        requestHandler._response = std::make_pair("application/xml", std::vector<char>(res.begin(), res.end()));
      }
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myInterfaceClient.sumUpSomeData(3, {23, 12, 20}, 5) == 23);
        CHECK(requestHandler._seenRequest._functionName == "sumUpSomeData");
        CHECK(requestHandler._seenRequest._input == "<t>23:12:20</t>");
        CHECK(requestHandler._seenRequest._params ==
              (std::vector<std::pair<std::string,std::string>>{{"someUInt", "3"}, {"otherUInt", "5"}}));
      }
    }
  }
}

TEST_CASE("Call more complex function 3 with RequestClient", "[RequestClient]")
{
  GIVEN("A RequestServer") {
    RequestHandlerMock requestHandler;
    MyInterfaceClient myInterfaceClient(requestHandler);
    WHEN("A request comes in") {
      {
        const std::string res("<returnType time=\"1:02:03\" someLetter=\"b\" someUnsignedShortInt=\"123\" moreTime=\"23:12:20\"/>");
        requestHandler._response = std::make_pair("application/xml", std::vector<char>(res.begin(), res.end()));
      }
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myInterfaceClient.constructSomeMoreComplexTestData(1, 2, 3, 'b', 123, {23, 12, 20}) ==
              SomeMoreComplexTestData(Time{1, 2, 3}, 'b', 123, Time{23, 12, 20}));
        CHECK(requestHandler._seenRequest._functionName == "constructSomeMoreComplexTestData");
        CHECK(requestHandler._seenRequest._input == "<time2>23:12:20</time2>");
        CHECK(requestHandler._seenRequest._params ==
              (std::vector<std::pair<std::string,std::string>>{
                {"hour1", "1"}, {"minute1", "2"}, {"second1", "3"}, {"someLetter", "b"}, {"someUnsignedShortInt", "123"}}));
      }
    }
  }
}

TEST_CASE("Call more complex function 4 with RequestClient (UrlSerialized structures)", "[RequestClient]")
{
  GIVEN("A RequestServer") {
    RequestHandlerMock requestHandler;
    MyInterfaceClient myInterfaceClient(requestHandler);
    WHEN("A request comes in") {
      {
        const std::string res(
              "<returnType time=\"1:02:03\" someLetter=\"b\" someUnsignedShortInt=\"123\" moreTime=\"23:12:20\"/>");
        requestHandler._response = std::make_pair("application/xml", std::vector<char>(res.begin(), res.end()));
      }
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myInterfaceClient.constructFromComplexParams(SomeMoreComplexTestData(Time{2, 2, 2}, 'b', 2, Time{5, 5, 5}),
                                                           SomeMoreComplexTestData(Time{3, 3, 3}, 'c', 3, Time{6, 6, 6}),
                                                           SomeMoreComplexTestData(Time{4, 4, 4}, 'd', 4, Time{7, 7, 7})) ==
              SomeMoreComplexTestData(Time{1, 2, 3}, 'b', 123, Time{23, 12, 20}));
        CHECK(requestHandler._seenRequest._functionName == "constructFromComplexParams");
        CHECK(requestHandler._seenRequest._input ==
              "<input time=\"2:02:02\" someLetter=\"b\" someUnsignedShortInt=\"2\" moreTime=\"5:05:05\"/>");
        CHECK(requestHandler._seenRequest._params ==
              (std::vector<std::pair<std::string,std::string>>{
                 {"p1.time", "3:03:03"}, {"p1.someLetter", "c"}, {"p1.someUnsignedShortInt", "3"}, {"p1.moreTime", "6:06:06"},
                 {"p2.time", "4:04:04"}, {"p2.someLetter", "d"}, {"p2.someUnsignedShortInt", "4"}, {"p2.moreTime", "7:07:07"}}));
      }
    }
  }
}
