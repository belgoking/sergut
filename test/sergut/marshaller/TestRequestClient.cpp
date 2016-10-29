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
#include "sergut/marshaller/RequestClient.h"
#include "sergut/marshaller/TestSupportClassesMarshaller.h"

#include <sstream>
#include <iomanip>



class MyImplementation: public sergut::marshaller::RequestClient, public MyInterface {
public:
  MyImplementation(sergut::marshaller::RequestClient::RequestHandler& requestHandler)
    : sergut::marshaller::RequestClient(requestHandler)
  {
    sergut::marshaller::RequestClient& cnt = *static_cast<sergut::marshaller::RequestClient*>(this);
    MyInterface::initialize(cnt);
  }

  SomeMoreComplexTestData
  constructSomeMoreComplexTestData(std::uint8_t hour1, const std::uint8_t minute1, const std::uint8_t& second1,
                                   char someLetter, std::uint16_t someUnsignedShortInt,
                                   const Time& time2) const override
  {
    return call<SomeMoreComplexTestData>("constructSomeMoreComplexTestData",
                                         hour1, minute1, second1, someLetter,
                                         someUnsignedShortInt, time2);
  }
  uint32_t sumUpSomeData(std::uint32_t someUInt, const Time& t, uint32_t otherUInt) const override
  {
    return call<uint32_t>("sumUpSomeData", someUInt, t, otherUInt);
  }
  uint32_t empty() const override {
    return call<uint32_t>("empty");
  }
};

class RequestHandler: public sergut::marshaller::RequestClient::RequestHandler
{
public:
  std::pair<std::string,std::vector<char>> handleRequest(const sergut::marshaller::RequestClient::Request& request) const override
  {
    _seenRequest = request;
    return _response;
  }
public:
  mutable sergut::marshaller::RequestClient::Request _seenRequest;
  std::pair<std::string,std::vector<char>> _response;
};

TEST_CASE("Call simple function 1 with RequestClient", "[RequestClient]")
{
  GIVEN("A RequestServer") {
    RequestHandler requestHandler;
    MyImplementation myImplementation(requestHandler);
    WHEN("A request comes in") {
      {
        const std::string res("<rt>42</rt>");
        requestHandler._response = std::make_pair("application/xml", std::vector<char>(res.begin(), res.end()));
      }
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myImplementation.empty() == 42);
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
    RequestHandler requestHandler;
    MyImplementation myImplementation(requestHandler);
    WHEN("A request comes in") {
      {
        const std::string res("<returnUInt32>23</returnUInt32>");
        requestHandler._response = std::make_pair("application/xml", std::vector<char>(res.begin(), res.end()));
      }
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myImplementation.sumUpSomeData(3, {23, 12, 20}, 5) == 23);
        CHECK(requestHandler._seenRequest._functionName == "sumUpSomeData");
        CHECK(requestHandler._seenRequest._input == "<t>23:12:20</t>");
        CHECK(requestHandler._seenRequest._params ==
              (std::vector<std::pair<std::string,std::string>>{{"someUInt", "3"}, {"otherUInt", "5"}}));
      }
    }
  }
}

TEST_CASE("Call simple function 3 with RequestClient", "[RequestClient]")
{
  GIVEN("A RequestServer") {
    RequestHandler requestHandler;
    MyImplementation myImplementation(requestHandler);
    WHEN("A request comes in") {
      {
        const std::string res("<returnType time=\"1:02:03\" someLetter=\"b\" someUnsignedShortInt=\"123\" moreTime=\"23:12:20\"/>");
        requestHandler._response = std::make_pair("application/xml", std::vector<char>(res.begin(), res.end()));
      }
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myImplementation.constructSomeMoreComplexTestData(1, 2, 3, 'b', 123, {23, 12, 20}) ==
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
