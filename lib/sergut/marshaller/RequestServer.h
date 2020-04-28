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

#include "sergut/UrlDeserializer.h"
#include "sergut/XmlDeserializer.h"
#include "sergut/XmlSerializer.h"
#include "sergut/marshaller/UnsupportedFormatException.h"
#include "sergut/marshaller/UnknownFunctionException.h"
#include "sergut/misc/ConstStringRef.h"

#include <functional>
#include <type_traits>

#if 0 // Not implemented, yet
namespace sergut {
namespace marshaller {

class RequestServer {
public:
  class Request {
  public:
    virtual ~Request() { }
    virtual sergut::misc::ConstStringRef getFunctionName() const = 0;
    virtual sergut::misc::ConstStringRef getParameter(const sergut::misc::ConstStringRef& parameterName) const = 0;
    virtual std::vector<std::pair<std::string,std::string>> getParameters() const = 0;
    virtual sergut::misc::ConstStringRef getInputDataContentType() const = 0;
    virtual sergut::misc::ConstStringRef getInputData() const = 0;
    virtual sergut::misc::ConstStringRef getOutputDataContentType() const = 0;
  };

  class Parameter {
  public:
    Parameter(const std::string& parameterName) : _parameterName(parameterName) { }
    template<typename T>
    T convert(const Request& request, UrlDeserializer& urlDeserializer) const {
      (void)request;
      return urlDeserializer.deserializeData<T>(_parameterName.c_str());
    }
    std::string _parameterName;
  };

  class Input {
  public:
    Input(const std::string& outerTagName)
      : _outerTagName(outerTagName)
    { }
    template<typename T>
    T convert(const Request& request, UrlDeserializer& urlDeserializer) const {
      (void)urlDeserializer;
      if(request.getInputDataContentType() == "application/xml") {
        sergut::XmlDeserializer des(request.getInputData());
        return des.deserializeData<T>(_outerTagName.c_str());
      }
      throw sergut::marshaller::UnsupportedFormatException("Unsupported input format: " + request.getInputDataContentType().toString());
    }
    std::string _outerTagName;
  };

  class NestedInput {
  public:
    /// \param xmlValueType the type of nesting that is desired this is mainly relevant for simple data types and
    ///        string serializable types as they can be serialized as attributes or as simpleChild
    NestedInput(const std::string& outerTagName, const std::string& innerTagName, const XmlValueType xmlValueType)
      : _outerTagName(outerTagName), _innerTagName(innerTagName), _xmlValueType(xmlValueType)
    { }
    template<typename T>
    T convert(const Request& request, UrlDeserializer& urlDeserializer) const {
      (void)urlDeserializer;
      if(request.getInputDataContentType() == "application/xml") {
        sergut::XmlDeserializer des(request.getInputData());
        return des.deserializeNestedData<T>(_outerTagName.c_str(), _innerTagName.c_str(), _xmlValueType);
      }
      throw sergut::marshaller::UnsupportedFormatException("Unsupported input format: " + request.getInputDataContentType().toString());
    }
    std::string _outerTagName;
    std::string _innerTagName;
    XmlValueType _xmlValueType;
  };

  RequestServer() { }

  template<typename Cls, typename RetT, typename ...FunArgs, typename ...Converters>
  void add(const std::string& funName, const Cls* cls, const std::string& returnWrapperName, RetT(Cls::*fun)(FunArgs ...funArgs) const, Converters&& ...converters)
  {
    static_assert(sizeof...(FunArgs) == sizeof...(Converters), "One converter is required per function argument");
    std::function<std::string(const Request&)> foo = (
          [=](const Request& request) {
             UrlDeserializer urlDeserializer(request.getParameters());
             RetT retVal = (cls->*fun)((converters.template convert<typename std::decay<FunArgs>::type>(request, urlDeserializer))...);
             if(request.getOutputDataContentType() == "application/xml") {
               sergut::XmlSerializer ser;
               ser.serializeData(returnWrapperName.c_str(), retVal);
               return ser.str();
             }
             throw sergut::marshaller::UnsupportedFormatException("No support for output format: " + request.getOutputDataContentType().toString());
    });
    mappings.insert(std::make_pair(funName, foo));
  }
  template<typename Cls>
  void setHandler(std::unique_ptr<Cls>&& serverFunction) {
    Cls* cls = &*serverFunction;
//    _i = std::move(serverFunction);
    cls->initialize(*this);
  }

  std::string call(const Request& request) {
    auto i = mappings.find(request.getFunctionName().toString());
    if(i == mappings.end()) {
      std::cout << request.getFunctionName() << " gibt es nicht" << std::endl;
      throw sergut::marshaller::UnknownFunctionException(request.getFunctionName().toString());
    }
    std::cout << "rufe " << request.getFunctionName() << " auf" << std::endl;
    return i->second(request);
  }

private:
  std::map<std::string, std::function<std::string(const Request&)>> mappings;
};

}
}
#endif
