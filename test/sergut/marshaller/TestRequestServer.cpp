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

#include "../TestSupportClasses.h"
#include "sergut/marshaller/RequestServer.h"

#include <sstream>
#include <iomanip>


class MyInterface {
public:
  virtual ~MyInterface() { }

  virtual
  SomeMoreComplexTestData
  constructSomeMoreComplexTestData(std::uint8_t hour1, const std::uint8_t minute1, const std::uint8_t& second1,
                                   char someLetter, std::uint16_t someUnsignedShortInt,
                                   std::uint8_t hour2, std::uint8_t minute2, std::uint8_t second2) const = 0;
  virtual
  uint32_t sumUpSomeData(std::uint32_t someUInt, const Time& t, uint32_t otherUInt) const = 0;

  virtual
  uint32_t empty() const = 0;

  template<typename Server>
  void initialize(Server& server) {
//    server.add(std::string("constructSomeMoreComplexTestData"), this, std::string("returnType"),
//               &MyInterface::constructSomeMoreComplexTestData,
//               typename Server::Parameter("hour1"),
//               typename Server::Parameter("minute1"),
//               typename Server::Parameter("second1"),
//               typename Server::Parameter("someLetter"),
//               typename Server::Parameter("someUnsignedShortInt"),
//               typename Server::Parameter("hour2"),
//               typename Server::Parameter("minute2"),
//               typename Server::Parameter("second2")
//               );
    server.add("sumUpSomeData", this, "returnUInt32",
               &MyInterface::sumUpSomeData,
               typename Server::Parameter("someUInt"),
               typename Server::Input("t"),
               typename Server::Parameter("otherUInt")
               );
    server.add("empty", this, "rt", &MyInterface::empty);
  }
};
class MyImplementation: public sergut::marshaller::RequestServer, public MyInterface {
public:
  MyImplementation() {
    sergut::marshaller::RequestServer& srv = *static_cast<sergut::marshaller::RequestServer*>(this);
    MyInterface::initialize(srv);
  }

  SomeMoreComplexTestData
  constructSomeMoreComplexTestData(std::uint8_t hour1, const std::uint8_t minute1, const std::uint8_t& second1,
                                   char someLetter, std::uint16_t someUnsignedShortInt,
                                   std::uint8_t hour2, std::uint8_t minute2, std::uint8_t second2) const override
  {
    return SomeMoreComplexTestData(Time(hour1, minute1, second1), someLetter,
                                   someUnsignedShortInt,
                                   Time(hour2, minute2, second2));
  }
  uint32_t sumUpSomeData(std::uint32_t someUInt, const Time& t, uint32_t otherUInt) const override
  {
    return someUInt + t.getInternalValue() + otherUInt;
  }
  uint32_t empty() const override {
    return 73;
  }
};

struct Request: public sergut::marshaller::RequestServer::Request {
public:
  Request(std::string aFunctionName,
          std::map<std::string, std::string> aParameters,
          std::string aInputData)
    : functionName(aFunctionName)
    , parameters(aParameters)
    , inputData(aInputData)
  { }
  sergut::misc::ConstStringRef getFunctionName() const override { return sergut::misc::ConstStringRef(functionName); }
  sergut::misc::ConstStringRef getParameter(const sergut::misc::ConstStringRef& parameterName) const override {
    const auto it = parameters.find(parameterName.toString());
    if(it == parameters.end()) {
      return sergut::misc::ConstStringRef();
    }
    return sergut::misc::ConstStringRef(it->second);
  }
  sergut::misc::ConstStringRef getInputDataContentType() const override { return sergut::misc::ConstStringRef("application/xml"); }
  sergut::misc::ConstStringRef getInputData() const override { return sergut::misc::ConstStringRef(inputData); }
  sergut::misc::ConstStringRef getOutputDataContentType() const override { return sergut::misc::ConstStringRef("application/xml"); }

public:
  std::string functionName;
  std::map<std::string, std::string> parameters;
  std::string inputData;
};

TEST_CASE("Call some simple functions", "[RequestServer]")
{
  GIVEN("A RequestServer") {
    MyImplementation myImplementation;
    WHEN("A request comes in") {
      Request request{ "sumUpSomeData", {{"someUInt", "3"}, {"otherUInt", "5"}}, "<t>23:12:20</t>" };
      THEN("Deserialization, marshalling & unmarshalling works") {
        CHECK(myImplementation.call(request) == "<returnUInt32>231228</returnUInt32>");
      }
    }
  }
}
