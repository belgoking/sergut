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

#pragma once

#include "sergut/TestSupportClasses.h"

#include "sergut/marshaller/RequestClient.h"
#include "sergut/marshaller/RequestServer.h"

class MyInterface {
public:
  virtual ~MyInterface() { }

  virtual
  SomeMoreComplexTestData
  constructSomeMoreComplexTestData(const std::uint8_t hour1, const std::uint8_t minute1, const std::uint8_t second1,
                                   const char someLetter, const std::uint16_t someUnsignedShortInt, const Time& time2) const = 0;
  virtual
  uint32_t sumUpSomeData(std::uint32_t someUInt, const Time& t, uint32_t otherUInt) const = 0;

  virtual
  uint32_t empty() const = 0;

  template<typename Server>
  void initialize(Server& server) {
    server.add(std::string("constructSomeMoreComplexTestData"), this, std::string("returnType"),
               &MyInterface::constructSomeMoreComplexTestData,
               typename Server::Parameter("hour1"),
               typename Server::Parameter("minute1"),
               typename Server::Parameter("second1"),
               typename Server::Parameter("someLetter"),
               typename Server::Parameter("someUnsignedShortInt"),
               typename Server::Input("time2")
               );
    server.add("sumUpSomeData", this, "returnUInt32",
               &MyInterface::sumUpSomeData,
               typename Server::Parameter("someUInt"),
               typename Server::Input("t"),
               typename Server::Parameter("otherUInt")
               );
    server.add("empty", this, "rt", &MyInterface::empty);
  }
};


class MyInterfaceClient: public sergut::marshaller::RequestClient, public MyInterface {
public:
  MyInterfaceClient(sergut::marshaller::RequestClient::RequestHandler& requestHandler)
    : sergut::marshaller::RequestClient(requestHandler)
  {
    sergut::marshaller::RequestClient& cnt = *static_cast<sergut::marshaller::RequestClient*>(this);
    MyInterface::initialize(cnt);
  }

  SomeMoreComplexTestData
  constructSomeMoreComplexTestData(const std::uint8_t hour1, const std::uint8_t minute1, const std::uint8_t second1,
                                   const char someLetter, const std::uint16_t someUnsignedShortInt,
                                   const Time& time2) const override
  {
    return call<SomeMoreComplexTestData>("constructSomeMoreComplexTestData", hour1, minute1, second1, someLetter, someUnsignedShortInt, time2);
  }
  uint32_t sumUpSomeData(std::uint32_t someUInt, const Time& t, uint32_t otherUInt) const override
  {
    return call<uint32_t>("sumUpSomeData", someUInt, t, otherUInt);
  }
  uint32_t empty() const override {
    return call<uint32_t>("empty");
  }
};


class RequestHandlerMock: public sergut::marshaller::RequestClient::RequestHandler
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


class MyInterfaceServer: public sergut::marshaller::RequestServer, public MyInterface {
public:
  MyInterfaceServer() {
    sergut::marshaller::RequestServer& srv = *static_cast<sergut::marshaller::RequestServer*>(this);
    MyInterface::initialize(srv);
  }

  SomeMoreComplexTestData
  constructSomeMoreComplexTestData(const std::uint8_t hour1, const std::uint8_t minute1, const std::uint8_t second1,
                                   const char someLetter, const std::uint16_t someUnsignedShortInt,
                                   const Time& time2) const override
  {
    return SomeMoreComplexTestData(Time(hour1, minute1, second1), someLetter,
                                   someUnsignedShortInt, time2);
  }
  uint32_t sumUpSomeData(std::uint32_t someUInt, const Time& t, uint32_t otherUInt) const override
  {
    return someUInt + t.getInternalValue() + otherUInt;
  }
  uint32_t empty() const override {
    return 73;
  }
};


struct RequestMock: public sergut::marshaller::RequestServer::Request {
public:
  RequestMock(std::string aFunctionName,
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
