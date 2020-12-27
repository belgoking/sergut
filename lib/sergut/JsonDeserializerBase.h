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

#include "sergut/DeserializerBase.h"
#include "sergut/ParsingException.h"
#include "sergut/SerializationException.h"
#include "sergut/Util.h"
#include "sergut/misc/ReadHelper.h"

#define RAPIDJSON_ASSERT(x) \
    if(!(x)) { \
        throw sergut::SerializationException("Assertion failed: " #x); \
    }

#include <rapidjson/document.h>

#include <cassert>
#include <cstring>
#include <list>
#include <memory>
#include <limits>
#include <set>
#include <string>
#include <sstream>
#include <vector>


namespace sergut {

/**
 * \brief Base class for a deserializer that uses a DOM-Like parser for JSON deserialization.
 */
template<typename Dialect>
class JsonDeserializerBase : public DeserializerBase
{
public:
  class ErrorContext;

public:
  JsonDeserializerBase(const std::string& json)
    : _jsonDocument(new rapidjson::Document)
    , _currentElement(_jsonDocument.get())
  {
    _jsonDocument->Parse(json.c_str());
  }

  /**
   * \brief Deserialize data into type \c DT
   * \tparam DT The type into which the JSON should be deserialized.
   */
  template<typename DT>
  DT deserializeData() {
    if(_jsonDocument.get() == nullptr) {
      throw ParsingException("A parser object MUST NOT be used more than once");
    }
    DT data;
    try {
      static_cast<Dialect*>(this)->deserializeValue(data);
    } catch(...) {
      _currentElement = nullptr;
      _jsonDocument.reset();
      throw;
    }
    _currentElement = nullptr;
    _jsonDocument.reset();

    return data;
  }

  template<typename DT>
  JsonDeserializerBase& operator&(const NamedMemberForDeserialization<DT>& data) {
    auto currentElement = _currentElement;
    auto memberIt = currentElement->FindMember(data.name);
    if(memberIt == currentElement->MemberEnd() || memberIt->value.IsNull()) {
      if(data.mandatory) {
        throw ParsingException("Missing mandatory member");
      }
      return *this;
    }
    _currentElement = &memberIt->value;
    static_cast<Dialect*>(this)->deserializeValue(data.data);
    _currentElement = currentElement;
    _currentElement->RemoveMember(memberIt);
    return *this;
  }

  /// Not relevant for JSON
  JsonDeserializerBase& operator&(const ChildrenFollow&) { return *this; }

  /// Not relevant for JSON
  JsonDeserializerBase& operator&(const PlainChildFollows&) { return *this; }

  uint64_t getMatchingNumericType(const unsigned long long&) const {
    if(!_currentElement->IsUint64()) {
      throw new ParsingException("Expecting unsigned numeric type, but got something else");
    }
    return _currentElement->GetUint64();
  }
  uint64_t getMatchingNumericType(const unsigned  long int&) const { return getMatchingNumericType(static_cast<unsigned long long>(0)); }
  uint64_t getMatchingNumericType(const unsigned       int&) const { return getMatchingNumericType(static_cast<unsigned long long>(0)); }
  uint64_t getMatchingNumericType(const unsigned short int&) const { return getMatchingNumericType(static_cast<unsigned long long>(0)); }
  uint64_t getMatchingNumericType(const unsigned      char&) const { return getMatchingNumericType(static_cast<unsigned long long>(0)); }

  int64_t getMatchingNumericType(const signed long long&) const {
    if(!_currentElement->IsInt64()) {
      throw new ParsingException("Expecting numeric type, but got something else");
    }
    return _currentElement->GetInt64();
  }
  int64_t getMatchingNumericType(const signed  long int&) const { return getMatchingNumericType(static_cast<signed long long>(0)); }
  int64_t getMatchingNumericType(const signed       int&) const { return getMatchingNumericType(static_cast<signed long long>(0)); }
  int64_t getMatchingNumericType(const signed short int&) const { return getMatchingNumericType(static_cast<signed long long>(0)); }
  int64_t getMatchingNumericType(const signed      char&) const { return getMatchingNumericType(static_cast<signed long long>(0)); }

  double getMatchingNumericType(const double&) const {
    if(!_currentElement->IsDouble()) {
      throw new ParsingException("Expecting double type, but got something else");
    }
    return _currentElement->GetDouble();
  }
  double getMatchingNumericType(const float&) const { return getMatchingNumericType(static_cast<double>(0)); }

  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, void>::type
  deserializeValue(T& data) {
    auto intValue = getMatchingNumericType(T());
    if(typeid(T) != typeid(intValue)) {
      if(intValue < std::numeric_limits<T>::min() || intValue > std::numeric_limits<T>::max()) {
        throw ParsingException("Number is not within the value range of datatype");
      }
    }
    data = intValue;
  }

  // String types
  void deserializeValue(bool& data) {
    if(_currentElement->IsBool()) {
      data = (*_currentElement).GetBool();
    }
    else if(_currentElement->IsInt()) {
      data = (*_currentElement).GetInt() != 0;
    }
    else {
      throw ParsingException("Expected Bool");
    }
  }

  // String types
  void deserializeValue(std::string& data) {
    if(!_currentElement->IsString()) {
      throw ParsingException("Expected String");
    }
    data = (*_currentElement).GetString();
  }

  void deserializeValue(char& data) {
    std::string tmp;
    deserializeValue(tmp);
    if(tmp.size() != 1) {
      throw ParsingException("Expecting string of size 1");
    }
    data = tmp[0];
  }

  void deserializeValue(char*& data) = delete;

  template<typename Collection>
  static void insertIntoCollection(Collection& collection, typename Collection::value_type&& data) {
    collection.push_back(std::move(data));
  }

  template<typename CDT>
  static void insertIntoCollection(std::set<CDT>& collection, CDT&& data) {
    collection.insert(std::move(data));
  }

  // Containers as members
  template<typename Collection>
  void deserializeCollection(Collection& data)
  {
    if(!_currentElement->IsArray()) {
      throw ParsingException("Expecting Collection, but got something else");
    }
    const auto currentElement = _currentElement;
    while(!currentElement->Empty()) {
      _currentElement = currentElement->Begin();
      typename Collection::value_type el;
      static_cast<Dialect*>(this)->deserializeValue(el);
      insertIntoCollection(data, std::move(el));
      currentElement->Erase(currentElement->Begin());
    }
    _currentElement = currentElement;
  }

  template<typename CDT>
  void deserializeValue(std::vector<CDT>& data) {
    deserializeCollection(data);
  }

  template<typename CDT>
  void deserializeValue(std::list<CDT>& data) {
    deserializeCollection(data);
  }

  template<typename CDT>
  void deserializeValue(std::set<CDT>& data) {
    deserializeCollection(data);
  }

  // Members that can be converted to string
  template<typename DT>
  auto deserializeValue(DT& data)
  -> decltype(deserializeFromString(data, std::string()),void())
  {
    if(!_currentElement->IsString()) {
      throw ParsingException("Expecting String, but got something else");
    }
    deserializeFromString(data, std::string(_currentElement->GetString(), _currentElement->GetStringLength()));
  }

  template<typename DT>
  auto deserializeValue(DT& data)
  -> decltype(serialize(declval<Dialect&>(), data, static_cast<typename std::decay<DT>::type*>(nullptr)),void())
  {
    auto* el = _currentElement;
    serialize(static_cast<Dialect&>(*this), data, static_cast<typename std::decay<DT>::type*>(nullptr));
    _currentElement = el;
  }

  template<typename DT, typename DiscriminatorFunction, typename ...TypeOptions>
  JsonDeserializerBase& oneOf(DT& data, DiscriminatorFunction /*discriminatorFunction*/,
                              TypeOptions... typeOptions)
  {
    doOneOf(data, typeOptions...);
    return *this;
  }

protected:
  template<typename DT, typename DiscriminatorType, typename HandlerType>
  bool handleTypeOption(DT& data, ObjectByKeyValue<DiscriminatorType, HandlerType>& handler) {

    const auto memberIt = _currentElement->FindMember(handler.key);
    if(memberIt == _currentElement->MemberEnd()) {
      return false;
    }
    bool found = false;
    if(memberIt->value.IsString()) {
      found = std::strncmp(memberIt->value.GetString(), handler.value, memberIt->value.GetStringLength()) == 0;
    }
    else if(memberIt->value.IsInt64()) {
      found = std::to_string(memberIt->value.GetInt64()) == handler.value;
    }
    else if(memberIt->value.IsBool()) {
      static std::string trueStr("true");
      static std::string falseStr("false");
      if(memberIt->value.GetBool()) {
        found = trueStr == handler.value;
      } else {
        found = falseStr == handler.value;
      }
    }
    if(!found) {
      return false;
    }
    handler.handler.init(data);
    static_cast<Dialect*>(this)->deserializeValue(handler.handler.forwardToSubtype(data));
    return true;
  }

  template<typename DT, typename DiscriminatorType, typename HandlerType>
  bool handleTypeOption(DT& data, ObjectByKey<DiscriminatorType, HandlerType>& handler) {
    const auto memberIt = _currentElement->FindMember(handler.key);
    if(memberIt == _currentElement->MemberEnd()) {
      return false;
    }
    handler.handler.init(data);
    static_cast<Dialect*>(this)->deserializeValue(handler.handler.forwardToSubtype(data));
    return true;
  }

  template<typename DT>
  void doOneOf(DT&)
  {
    throw ParsingException("failed to find matching type for 'oneOf'");
  }

  template<typename DT, typename TypeOptionHead, typename ...TypeOptionsTail>
  void doOneOf(DT& data, TypeOptionHead& head, TypeOptionsTail&... tail)
  {
    if(!handleTypeOption(data, head)) {
      doOneOf(data, tail...);
    }
  }

protected:
  std::unique_ptr<typename rapidjson::Document> _jsonDocument;
  typename rapidjson::Value* _currentElement;
};

} // namespace sergut
