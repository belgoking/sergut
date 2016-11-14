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

#include "sergut/UrlSerializeToVector.h"
#include "sergut/XsdGenerator.h"
#include "sergut/marshaller/UnsupportedFormatException.h"
#include "sergut/marshaller/UnknownFunctionException.h"
#include "sergut/misc/ConstStringRef.h"

#include <type_traits>

namespace sergut {
namespace marshaller {

class RequestSpecificationGenerator {
public:
  class Parameter {
  public:
    Parameter(const std::string& parameterName) : _parameterName(parameterName) { }
    template<typename T>
    bool convert(std::string& xmlDescription, UrlSerializeToVector& urlSerializer) const {
      (void)xmlDescription;
      T dummy;
      urlSerializer.serializeData<T>(_parameterName.c_str(), dummy);
      return true;
    }
    std::string _parameterName;
  };

  class Input {
  public:
    Input(const std::string& outerTagName)
      : _outerTagName(outerTagName)
    { }
    template<typename T>
    bool convert(std::string& xmlDescription, UrlSerializeToVector& urlSerializer) const {
      (void)urlSerializer;
      std::ostringstream out;
      XsdGenerator xsdGenerator(out);
      xsdGenerator.serializeData<T>(_outerTagName);
      xmlDescription = out.str();
      return true;
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
    bool convert(std::string& xmlDescription, UrlSerializeToVector& urlSerializer) const {
      (void)urlSerializer;
      std::ostringstream out;
      XsdGenerator xsdGenerator(out);
      xsdGenerator.serializeNestedData<T>(_outerTagName, _innerTagName, _xmlValueType);
      xmlDescription = out.str();
      return true;
    }
    std::string _outerTagName;
    std::string _innerTagName;
    XmlValueType _xmlValueType;
  };

  RequestSpecificationGenerator() { }

  template<typename Cls, typename RetT, typename ...FunArgs, typename ...Converters>
  void add(const std::string& funName, const Cls* cls, const std::string& returnWrapperName,
           RetT(Cls::*fun)(FunArgs ...funArgs) const, Converters&& ...converters)
  {
    (void)cls;
    (void)fun;
    static_assert(sizeof...(FunArgs) == sizeof...(Converters), "One converter is required per function argument");

    std::function<std::string(const std::string&)> foo = (
          [=](const std::string& contentType) {
             if(contentType == "application/xml") {
               std::string xmlDescription;
               UrlSerializeToVector urlSerializer;
               apply(converters.template convert<typename std::decay<FunArgs>::type>(xmlDescription, urlSerializer)...);

               std::ostringstream params;
               bool first = true;
               for(const auto& x: urlSerializer.getParams()) {
                 if(first) {
                   first = false;
                 } else {
                   params << "&";
                 }
                 params << x.first << "=";
               }

               std::ostringstream out;
               XsdGenerator xsdGenerator(out);
               xsdGenerator.serializeData<RetT>(returnWrapperName.c_str());
               return "Request: " + params.str() + "\n"
                   "Input: " + xmlDescription +
                   "Response: " + out.str();
             }
             throw sergut::marshaller::UnsupportedFormatException("No support for output format: " + contentType);
    });
    _requests.insert(std::make_pair(funName, foo));
  }

  std::string generateDocumentation(const std::string& function, const std::string& contentType) {
    auto i = _requests.find(function);
    if(i == _requests.end()) {
      std::cout << function << " gibt es nicht" << std::endl;
      throw sergut::marshaller::UnknownFunctionException(function);
    }
    return i->second(contentType);
  }

  const std::map<std::string, std::function<std::string(const std::string&)>> getRequests() const { return _requests; }

private:
  template<typename ...ARGS>
  static void apply(ARGS&& ...) { }
private:
  std::map<std::string, std::function<std::string(const std::string&)>> _requests;
};

}
}
