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

#include "sergut/XmlValueType.h"
#include "sergut/misc/ConstStringRef.h"
#include "sergut/misc/DataType.h"

#include <map>
#include <type_traits>
#include <vector>

#include <cassert>

namespace sergut {
namespace marshaller {
namespace detail {

class FunctionSignatureExtractor {
public:
  struct FunctionNameNParameterCount {
    std::string _functionName;
    std::size_t _parameterCount;
    bool operator<(const FunctionNameNParameterCount& rhs) const {
      return _functionName < rhs._functionName ||
          (_functionName == rhs._functionName && _parameterCount < rhs._parameterCount);
    }
  };
  struct SerializationDescription {
    std::string _outerTagName; ///< The outer tag name (for XML (nested and non-nested))
    std::string _innerTagName; ///< The inner tag name (for XML (nested only))
    XmlValueType _xmlValueType; ///< The XmlValueType for nested outer tags
  };

  struct InputData: SerializationDescription {
    std::size_t _parameterPosition = std::size_t(-1); ///< The position of the input data (as opposed to the parameters). -1 if there is no inputData for this function.
  };

  struct Variable {
    sergut::misc::DataType::Type _dataType;
    std::string _name;
  };

  struct FunctionSignature {
    SerializationDescription _returnData; ///< The description on what return data is expected
    InputData _inputData; ///< The input name,
    std::vector<Variable> _parameters; ///< The parameters (the value at position \c parameterNames[inputData.parameterPosition] will be ignored)
  };

  class Parameter {
  public:
    Parameter(const std::string& parameterName) : _parameterName(parameterName) { }
  private:
    friend class FunctionSignatureExtractor;
    void registerThis(FunctionSignature& sig, const std::size_t ordinalPosition, const sergut::misc::DataType::Type dataType) const {
      assert(sig._parameters.size() > ordinalPosition && sig._parameters[ordinalPosition]._name.empty());
      sig._parameters[ordinalPosition] = Variable{ dataType, _parameterName };
    }
  private:
    std::string _parameterName;
  };

  class Input {
  public:
    Input(const std::string& outerTagName)
      : _outerTagName(outerTagName)
    { }
  private:
    friend class FunctionSignatureExtractor;
    void registerThis(FunctionSignature& sig, const std::size_t ordinalPosition, const sergut::misc::DataType::Type dataType) const {
      (void)dataType; // not used, yet
      assert(sig._inputData._parameterPosition == std::size_t(-1));
      assert(sig._parameters.size() > ordinalPosition && sig._parameters[ordinalPosition]._name.empty());
      sig._inputData._parameterPosition = ordinalPosition;
      sig._inputData._outerTagName = _outerTagName;
    }
  private:
    std::string _outerTagName;
  };

  class NestedInput {
  public:
    /// \param xmlValueType the type of nesting that is desired this is mainly relevant for simple data types and
    ///        string serializable types as they can be serialized as attributes or as simpleChild
    NestedInput(const std::string& outerTagName, const std::string& innerTagName, const XmlValueType xmlValueType)
      : _outerTagName(outerTagName), _innerTagName(innerTagName), _xmlValueType(xmlValueType)
    { }
  private:
    friend class FunctionSignatureExtractor;
    void registerThis(FunctionSignature& sig, const std::size_t ordinalPosition, const sergut::misc::DataType::Type dataType) const {
      (void)dataType;
      assert(sig._inputData._parameterPosition == std::size_t(-1));
      assert(sig._parameters.size() > ordinalPosition && sig._parameters[ordinalPosition]._name.empty());
      sig._inputData._parameterPosition = ordinalPosition;
      sig._inputData._outerTagName = _outerTagName;
      sig._inputData._innerTagName = _innerTagName;
      sig._inputData._xmlValueType = _xmlValueType;
    }
  private:
    std::string _outerTagName;
    std::string _innerTagName;
    XmlValueType _xmlValueType;
  };

//  template<typename Parameter>
//  void addDescription(FunctionSignature& signature, const Parameter& parameter)
//  {
//    parameter.registerThis(signature, signature._parameterNames.size() - 1);
//  }

  void addDescription(FunctionSignature& signature, void(*)()) { (void)signature; }

  template<typename FirstFunArg, typename FirstConverter, typename ...FunArgs, typename ...Converters>
  void addDescription(FunctionSignature& signature,
                      void(*)(FirstFunArg firstFunArg, FunArgs ...funArgs),
                      FirstConverter&& firstConverter, const Converters& ...converters)
  {
    firstConverter.registerThis(signature, signature._parameters.size() - sizeof...(FunArgs) - 1,
                                sergut::misc::DataType::getDataType<FirstFunArg>());
    addDescription(signature, static_cast<void(*)(FunArgs ...funArgs)>(nullptr), converters...);

    assert(signature._inputData._parameterPosition == std::size_t(-1) ||
           (signature._inputData._parameterPosition < signature._parameters.size()
            && signature._parameters[signature._inputData._parameterPosition]._name.empty()));
  }


  template<typename Cls, typename RetT, typename ...FunArgs, typename ...Converters>
  void add(const std::string& funName,
           const Cls* cls, const std::string& returnWrapperName, RetT(Cls::*fun)(FunArgs ...funArgs) const,
           const Converters& ...converters)
  {
    (void)cls;
    (void)fun;
    static_assert(sizeof...(FunArgs) == sizeof...(Converters), "One converter is required per function argument");

    FunctionSignature signature;
    signature._parameters.resize(sizeof...(FunArgs));
    addDescription(signature, static_cast<void(*)(FunArgs ...funArgs)>(nullptr), converters...);
    signature._returnData._outerTagName = returnWrapperName;
    const bool hasInserted = _mappings.insert(std::make_pair(FunctionNameNParameterCount{funName, sizeof...(FunArgs)}, signature)).second;
    assert(hasInserted);
  }

public:
  // map the function name along with the amount of parameters to the parameter types
  std::map<FunctionNameNParameterCount, FunctionSignature> _mappings;
};

}
}
}
