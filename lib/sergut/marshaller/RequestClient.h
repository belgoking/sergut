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

#include "sergut/marshaller/InvalidCodePathException.h"
#include "sergut/marshaller/RemoteCallingException.h"
#include "sergut/marshaller/UnknownFunctionException.h"
#include "sergut/marshaller/detail/FunctionSignatureExtractor.h"
#include "sergut/misc/ConstStringRef.h"
#include "sergut/misc/ReadHelper.h"
#include "sergut/XmlDeserializer.h"
#include "sergut/XmlSerializer.h"

#include <type_traits>

namespace sergut {
namespace marshaller {
namespace {

template<typename T>
std::string toString(const T& d) {
  (void)d;
  // this implementation exists merely as some code paths
  // that should never be executed require it for template
  // instantication
  throw sergut::marshaller::InvalidCodePathException("Trying to read non-simple parameter from string");
}

inline std::string toString(const bool d)               { return std::to_string(d); }
inline std::string toString(const char d)               { return std::to_string(d); }
inline std::string toString(const unsigned char d)      { return std::to_string(d); }
inline std::string toString(const signed char d)        { return std::to_string(d); }
inline std::string toString(const short d)              { return std::to_string(d); }
inline std::string toString(const unsigned short d)     { return std::to_string(d); }
inline std::string toString(const int d)                { return std::to_string(d); }
inline std::string toString(const unsigned int d)       { return std::to_string(d); }
inline std::string toString(const long d)               { return std::to_string(d); }
inline std::string toString(const unsigned long d)      { return std::to_string(d); }
inline std::string toString(const long long d)          { return std::to_string(d); }
inline std::string toString(const unsigned long long d) { return std::to_string(d); }
inline std::string toString(const float d)              { return std::to_string(d); }
inline std::string toString(const double d)             { return std::to_string(d); }
inline std::string toString(const long double d)        { return std::to_string(d); }
inline std::string toString(const std::string& d)       { return d; }
inline std::string toString(const char* d)              { return std::string(d); }
}

class RequestClient {
public:
  typedef sergut::marshaller::detail::FunctionSignatureExtractor::Parameter Parameter;
  typedef sergut::marshaller::detail::FunctionSignatureExtractor::Input Input;
  typedef sergut::marshaller::detail::FunctionSignatureExtractor::NestedInput NestedInput;
public:
//  class Parameter {
//  public:
//    Parameter(const std::string& parameterName) : _parameterName(parameterName) { }
//  private:
//    friend class RequestClient;
//    void registerThis(FunctionSignature& sig, const std::size_t ordinalPosition) const {
//      assert(sig._inputData._parameterPosition != ordinalPosition);
//      assert(sig._parameterNames.size() > ordinalPosition && sig._parameterNames[ordinalPosition].empty());
//      sig._parameterNames[ordinalPosition] = _parameterName;
//    }
//  private:
//    std::string _parameterName;
//  };

//  class Input {
//  public:
//    Input(const std::string& outerTagName)
//      : _outerTagName(outerTagName)
//    { }
//  private:
//    friend class RequestClient;
//    void registerThis(FunctionSignature& sig, const std::size_t ordinalPosition) const {
//      assert(sig._inputData._parameterPosition == std::size_t(-1));
//      assert(sig._parameterNames.size() > ordinalPosition && sig._parameterNames[ordinalPosition].empty());
//      sig._inputData._parameterPosition = ordinalPosition;
//      sig._inputData._outerTagName = _outerTagName;
//    }
//  private:
//    std::string _outerTagName;
//  };

//  class NestedInput {
//  public:
//    /// \param xmlValueType the type of nesting that is desired this is mainly relevant for simple data types and
//    ///        string serializable types as they can be serialized as attributes or as simpleChild
//    NestedInput(const std::string& outerTagName, const std::string& innerTagName, const XmlValueType xmlValueType)
//      : _outerTagName(outerTagName), _innerTagName(innerTagName), _xmlValueType(xmlValueType)
//    { }
//  private:
//    friend class RequestClient;
//    void registerThis(FunctionSignature& sig, const std::size_t ordinalPosition) const {
//      assert(sig._inputData._parameterPosition == std::size_t(-1));
//      assert(sig._parameterNames.size() > ordinalPosition && sig._parameterNames[ordinalPosition].empty());
//      sig._inputData._parameterPosition = ordinalPosition;
//      sig._inputData._outerTagName = _outerTagName;
//      sig._inputData._innerTagName = _innerTagName;
//      sig._inputData._xmlValueType = _xmlValueType;
//    }
//  private:
//    std::string _outerTagName;
//    std::string _innerTagName;
//    XmlValueType _xmlValueType;
//  };

  struct Request {
    /// The name of the remote function
    std::string _functionName;
    /// The serialized parameters
    std::vector<std::pair<std::string, std::string>> _params;
    /// The content type of the input
    std::string _inputContentType;
    /// The request body
    std::string _input;
    /// The supported result types
    std::string _acceptContentType;
  };

  class RequestHandler {
  public:
    virtual ~RequestHandler();
    /**
     * @brief handleRequest Executes the remote function call with the serialized parameters
     * @return A pair with the content type of the result and the result bytes
     *
     * The implementer of a remote request backend has to implement this function and map
     * the different input types to the corresponding coding of the implemented calling
     * method and the result has to be mapped to the return type.
     */
    virtual std::pair<std::string,std::vector<char>> handleRequest(const Request& request) const = 0;
  };

  RequestClient(const RequestHandler& requestHandler)
    : _requestHandler(requestHandler)
  { }
  virtual ~RequestClient() { }

//  template<typename Parameter>
//  void addDescription(FunctionSignature& signature, const Parameter& parameter)
//  {
//    parameter.registerThis(signature, signature._parameterNames.size() - 1);
//  }

//  template<typename Parameter, typename ...Parameters>
//  void addDescription(FunctionSignature& signature, const Parameter& parameter, const Parameters& ...parameters)
//  {
//    parameter.registerThis(signature, signature._parameterNames.size() - sizeof...(parameters) - 1);
//    addDescription(signature, parameters...);
//  }

  template<typename Cls, typename RetT, typename ...FunArgs, typename ...Converters>
  void add(const std::string& funName,
           const Cls* cls, const std::string& returnWrapperName, RetT(Cls::*fun)(FunArgs ...funArgs) const,
           const Converters& ...converters)
  {
    (void)cls;
    _functionSignatures.add(funName, cls, returnWrapperName, fun, converters...);
//    (void)fun;
//    static_assert(sizeof...(FunArgs) == sizeof...(Converters), "One converter is required per function argument");

//    FunctionSignature signature;
//    addDescription(signature, converters...);
//    signature._returnData._outerTagName = returnWrapperName;
//    const bool hasInserted = _mappings.insert(std::make_pair({funName, sizeof(FunArgs)...}, signature)).second;
//    assert(hasInserted);
  }



  template<std::size_t ParamPos>
  void fillRequest(Request& request,
                   const detail::FunctionSignatureExtractor::FunctionSignature& signature) const
  {
    (void)request;
    (void)signature;
  }

  template<std::size_t ParamPos, typename FunArg, typename ...FunArgs>
  void fillRequest(Request& request,
                   const detail::FunctionSignatureExtractor::FunctionSignature& signature,
                   FunArg&& funArg,
                   FunArgs&& ...funArgs) const
  {
    // first fill the current parameter (we take advantage that the last parameter
    // is handled just the same as all other parameters
    if(signature._inputData._parameterPosition == ParamPos) {
      request._inputContentType = "application/xml";
      sergut::XmlSerializer ser;
      if(!signature._inputData._innerTagName.empty()) {
        ser.serializeNestedData(signature._inputData._outerTagName.c_str(),
                                signature._inputData._innerTagName.c_str(),
                                signature._inputData._xmlValueType,
                                funArg);
      } else {
        ser.serializeData(signature._inputData._outerTagName.c_str(), funArg);
      }
      request._input = ser.str();
    } else {
      request._params.push_back(std::make_pair(signature._parameters[ParamPos]._name, toString(funArg)));
    }

//    fillRequest<ParamPos>(request, signature, std::forward(funArg));
    // then we handle the remaining parameters
    fillRequest<ParamPos+1>(request, signature, funArgs...);
  }

  template<typename RetT, typename ...FunArgs>
  RetT call(const std::string& funName, FunArgs&& ...funArgs) const {
    const auto i =
        _functionSignatures._mappings.find(
          detail::FunctionSignatureExtractor::FunctionNameNParameterCount{funName, sizeof...(FunArgs)});
    if(i == _functionSignatures._mappings.end()) {
      std::cout << funName << " gibt es nicht" << std::endl;
      throw sergut::marshaller::UnknownFunctionException(funName);
    }
    const detail::FunctionSignatureExtractor::FunctionSignature& signature = i->second;
    std::cout << "rufe " << funName << " auf" << std::endl;
    Request request;
    request._functionName = funName;
    request._acceptContentType = "application/xml";
    // I know the following reserves one element too much in case we have input data but don't care
    request._params.reserve(signature._parameters.size());
    if(sizeof...(funArgs) > 0) {
//      fillRequest<0>(request, std::forward(funArgs)...);
      fillRequest<0>(request, signature, funArgs...);
    }
    std::pair<std::string, std::vector<char>> result = _requestHandler.handleRequest(request);
    if(!result.second.empty() && result.first != "application/xml") {
      throw RemoteCallingException("Unsupported return format: '" + result.first + "'");
    }
    XmlDeserializer deser(std::move(result.second));
    if(signature._returnData._innerTagName.empty()) {
      return deser.deserializeData<RetT>(signature._returnData._outerTagName.c_str());
    } else {
      return deser.deserializeNestedData<RetT>(
            signature._returnData._outerTagName.c_str(),
            signature._returnData._innerTagName.c_str(),
            signature._returnData._xmlValueType);
    }
  }

//private:
//  struct FunctionNameNParameterCount {
//    std::string _functionName;
//    std::size_t _parameterCount;
//    bool operator<(const FunctionNameNParameterCount& rhs) const {
//      return _functionName < rhs._functionName ||
//          (_functionName == rhs._functionName && _parameterCount < rhs._parameterCount);
//    }
//  };
//  struct SerializationDescription {
//    std::string _outerTagName; ///< The outer tag name (for XML (nested and non-nested))
//    std::string _innerTagName; ///< The inner tag name (for XML (nested only))
//    XmlValueType _xmlValueType; ///< The XmlValueType for nested outer tags
//  };

//  struct InputData: SerializationDescription {
//    std::size_t _parameterPosition = std::size_t(-1); ///< The position of the input data (as opposed to the parameters). -1 if there is no inputData for this function.
//  };

//  struct FunctionSignature {
//    SerializationDescription _returnData; ///< The description on what return data is expected
//    InputData _inputData; ///< The input name,
//    std::vector<std::string> _parameterNames; ///< The names of the parameters (the value at position \c parameterNames[inputData.parameterPosition] will be ignored)
//  };

private:
  const RequestHandler& _requestHandler;
  detail::FunctionSignatureExtractor _functionSignatures;
  // map the function name along with the amount of parameters to the parameter types
//  std::map<FunctionNameNParameterCount, FunctionSignature> _mappings;
};

}
}
