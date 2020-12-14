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

#include <functional>
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>

////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
template<typename T>
T createEmpty() {
  return T();
}

template<typename RetType, typename ...ArgTypes>
void callEmpty(RetType(*fn)(ArgTypes ...args)) {
  fn(createEmpty<ArgTypes>()...);
}

////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////
template<typename T>
T fromString(const std::string& str) {
  std::istringstream istr(str);
  T t;
  istr >> t;
  return t;
}

template<typename RetType, typename ...ArgTypes, typename ...ConfigTypes>
void callFromString(RetType(*fn)(ArgTypes ...args), ConfigTypes ...configTypes) {
  fn(fromString<ArgTypes>(configTypes)...);
}

int testFunction(int i, float f, char c, std::string str) {
  std::cout << "int: " << i << "\nfloat: " << f << "\nchar: " << c << "\nstring: " << str << "\n";
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
/// Store function pointers using variadic templates
////////////////////////////////////////////////////////////////////////////////
std::string fun1(int i, float f, char c, std::string str) {
  std::cout << __FUNCTION__ << " int: " << i << "\nfloat: " << f << "\nchar: " << c << "\nstring: " << str << "\n";
  return str;
}
int fun2(std::string str) {
  std::cout << __FUNCTION__ << " string: " << str << "\n";
  return str.size();
}




class Interface {
public:
//  virtual std::string fun1(int i, float f, char c, std::string str) {
//    std::cout << __FUNCTION__ << " int: " << i << "\nfloat: " << f << "\nchar: " << c << "\nstring: " << str << "\n";
//    return str;
//  }
//  virtual int fun2(std::string str) {
//    std::cout << __FUNCTION__ << " string: " << str << "\n";
//    return str.size();
//  }

  template<typename Server>
  void initialize(Server& server) {
    server.add("fun1", &fun1, "1234567890", "23.25", "abc", "Du Hund");
    server.add("fun2", &fun2, "abc");
  }

  template<typename Server>
  void initialize2(Server& server) {
    server.add("fun1", &fun1);
    server.add("fun2", &fun2);
  }
};

class Server {
public:
  Server() { }

  template<typename RetT, typename ...FunArgs, typename ...Params>
  void add(const std::string& funName, RetT(*fun)(FunArgs ...funArgs), Params ...params)
  {
    std::function<void()> foo = ([=](){ (*fun)(fromString<FunArgs>(params)...); });
    mappings.insert(std::make_pair(funName, foo));
  }
  void instantiateHandler() {
    _i.reset(new Interface);
    _i->initialize(*this);
  }

  void call(const std::string& str) {
    auto i = mappings.find(str);
    if(i == mappings.end()) {
      std::cout << "XXXX mit " << str << std::endl;
      return;
    }
    i->second();
  }

private:
  std::unique_ptr<Interface> _i;
  std::map<std::string, std::function<void()>> mappings;
};


class Server2 {
public:
  Server2() { }

  template<typename RetT, typename ...FunArgs, typename ...Params>
  void add(const std::string& funName, RetT(*fun)(FunArgs ...funArgs))
  {
    std::function<void(const std::string&)> foo = ([=](const std::string& str){ (*fun)(fromString<FunArgs>(str)...); });
    mappings.insert(std::make_pair(funName, foo));
  }

  void instantiateHandler() {
    _i.reset(new Interface);
    _i->initialize2(*this);
  }

  void call(const std::string& funName, const std::string& str) {
    auto i = mappings.find(funName);
    if(i == mappings.end()) {
      std::cout << "XXXX mit " << funName << std::endl;
      return;
    }
    i->second(str);
  }

private:
  std::unique_ptr<Interface> _i;
  std::map<std::string, std::function<void(const std::string&)>> mappings;
};


////////////////////////////////////////////////////////////////////////////////
/// Store member function pointers using variadic templates
////////////////////////////////////////////////////////////////////////////////
class InterfaceBase {
public:
  virtual ~InterfaceBase() { }
};

class Interface21: public InterfaceBase {
public:
  virtual ~Interface21() {
    std::cout << __FUNCTION__ << " DESTRUCTION" << std::endl;
  }
  virtual std::string fun1(int i, float f, char c, std::string str) const {
    std::cout << __FUNCTION__ << " int: " << i << "\nfloat: " << f << "\nchar: " << c << "\nstring: " << str << "\n";
    return str;
  }
  virtual int fun2(std::string str) const {
    std::cout << __FUNCTION__ << " string: " << str << "\n";
    return str.size();
  }

  template<typename Server>
  void initialize(Server& server) {
    server.add("fun1", this, &Interface21::fun1);
    server.add("fun2", this, &Interface21::fun2);
  }
};

class Server21 {
public:
  Server21() { }

  template<typename Cls, typename RetT, typename ...FunArgs>
  void add(const std::string& funName, const Cls* cls, RetT(Interface21::*fun)(FunArgs ...funArgs) const)
  {
    std::function<void(const std::string&)> foo = ([=](const std::string& str){ (cls->*fun)(fromString<FunArgs>(str)...); });
    mappings.insert(std::make_pair(funName, foo));
  }
  template<typename Cls>
  void setHandler(std::unique_ptr<Cls>&& serverFunction) {
    Cls* cls = &*serverFunction;
    _i = std::move(serverFunction);
    cls->initialize(*this);
  }

  void call(const std::string& funName, const std::string& str) {
    auto i = mappings.find(funName);
    if(i == mappings.end()) {
      std::cout << funName << " gibt es nicht" << std::endl;
      return;
    }
    std::cout << "rufe " << funName << " mit '" << str << "'" << std::endl;
    i->second(str);
  }

private:
  std::unique_ptr<InterfaceBase> _i;
  std::map<std::string, std::function<void(const std::string&)>> mappings;
};

////////////////////////////////////////////////////////////////////////////////
/// HttpServer with Marshalling
////////////////////////////////////////////////////////////////////////////////
class InterfaceHttp: public InterfaceBase {
public:
  virtual ~InterfaceHttp() {
    std::cout << __FUNCTION__ << " DESTRUCTION" << std::endl;
  }

  virtual std::string fun1(int i, float f, char c, std::string str) const {
    std::cout << __FUNCTION__ << " int: " << i << "\nfloat: " << f << "\nchar: " << c << "\nstring: " << str << "\n";
    return str;
  }

  virtual int fun2(std::string str) const {
    std::cout << __FUNCTION__ << " string: " << str << "\n";
    return str.size();
  }

  template<typename Server>
  void initialize(Server& server) {
    server.add("fun1", this, &InterfaceHttp::fun1,
               typename Server::Parameter("int"),
               typename Server::Parameter("float"),
               typename Server::Input(),
               typename Server::Parameter("string"));
    server.add("fun2", this, &InterfaceHttp::fun2, typename Server::Input());
  }
};

class HttpServer {
public:
  class HttpRequest {
  public:
    HttpRequest() = default;
    HttpRequest(const std::string& path, const std::map<std::string, std::string>& queryParams, const std::string& body)
      : _path(path)
      , _queryParams(queryParams)
      , _body(body)
    { }
    std::string _path;
    std::map<std::string, std::string> _queryParams;
    std::string _body;
  };

  class Parameter {
  public:
    Parameter(const std::string& parameterName) : _parameterName(parameterName) { }
    template<typename T>
    T convert(const HttpRequest& request) const {
      auto it = request._queryParams.find(_parameterName);
      if(it == request._queryParams.end()) {
        return T();
      }
      std::istringstream istr(it->second);
      T t;
      istr >> t;
      return t;
    }
    std::string _parameterName;
  };

  class Input {
  public:
    template<typename T>
    T convert(const HttpRequest& request) const {
      std::istringstream istr(request._body);
      T t;
      istr >> t;
      return t;
    }
  };

  HttpServer() { }

  template<typename Cls, typename RetT, typename ...FunArgs, typename ...Converters>
  void add(const std::string& funName, const Cls* cls, RetT(Cls::*fun)(FunArgs ...funArgs) const, Converters&& ...converters)
  {
    static_assert(sizeof...(FunArgs) == sizeof...(Converters), "One converter is required per function argument");
    std::function<void(const HttpRequest&)> foo = (
          [=](const HttpRequest& request){
             (cls->*fun)((converters.template convert<FunArgs>(request))...);
    });
    mappings.insert(std::make_pair(funName, foo));
  }
  template<typename Cls>
  void setHandler(std::unique_ptr<Cls>&& serverFunction) {
    Cls* cls = &*serverFunction;
    _i = std::move(serverFunction);
    cls->initialize(*this);
  }

  void call(const HttpRequest& request) {
    auto i = mappings.find(request._path);
    if(i == mappings.end()) {
      std::cout << request._path << " gibt es nicht" << std::endl;
      return;
    }
    std::cout << "rufe " << request._path << " auf" << std::endl;
    i->second(request);
  }

private:
  std::unique_ptr<InterfaceBase> _i;
  std::map<std::string, std::function<void(const HttpRequest&)>> mappings;
};

TEST_CASE("some test for hypercall", "[unicode]")
{
  // hypercall
  // hyperEVA++
  // EVA++
  // evaRemote
  // remoteEva

//  callEmpty(&testFunction);
//  callFromString(&testFunction, "1", "2", "3", "fourth");
//  Server s;
//  s.instantiateHandler();
//  s.call("GIBTSNICHT");
//  s.call("fun1");
//  s.call("fun2");

//  Server2 s2;
//  s2.instantiateHandler();
//  s2.call("GIBTSNICHT", "123.123");
//  s2.call("fun1", "123.123");
//  s2.call("fun2", "123.123");

//  Server21 s21;
//  s21.setHandler(std::unique_ptr<Interface21>(new Interface21));
//  s21.call("GIBTSNICHT", "123.123");
//  s21.call("fun1", "123.123");
//  s21.call("fun2", "123.123");

  HttpServer http;
  http.setHandler(std::unique_ptr<InterfaceHttp>(new InterfaceHttp));
  HttpServer::HttpRequest request;
  request._body = "achar";
  request._queryParams = std::map<std::string,std::string>{{"int", "99"},{"float", "23.23"},{"string", "this_is a string"}};
  request._path = "GIBTSNICHT";
  http.call(request);
  request._path = "fun1";
  http.call(request);
  request._path = "fun2";
  http.call(request);


//  const std::vector<unicode::Utf32Char> invalidCodepoints{
//    0x110000, // too large
//    0xDC00, // invalid range start
//    0xDFFF, // invalid range end
//  };
//  for(unicode::Utf32Char c: invalidCodepoints) {
//    std::ostringstream txt;
//    txt << "The non-existent code point 0x" << std::hex << c;
//    GIVEN(txt.str()) {
//      WHEN("It is encoded to UTF-16BE") {
//        THEN("The coder must return an error") {
//          std::string outString;
//          const unicode::ParseResult r = unicode::Utf8Codec::appendChar(outString, c);
//          CHECK(r == unicode::ParseResult::InvalidCharacter);
//          CHECK(outString.empty());
//        }
//      }
//    }
//  }
}

