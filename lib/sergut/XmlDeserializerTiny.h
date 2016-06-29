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
#include "sergut/MemberDeserializer.h"
#include "sergut/ParsingException.h"
#include "sergut/SerializerBase.h"
#include "sergut/Util.h"

#include <tinyxml.h>

#include <string>
#include <vector>
#include <list>
#include <set>
#include <cassert>
#include <iostream>

#include <cstring>

/*
 * <foo>
 *   <bar baz="1">val</bar>
 *   <bam>val</bam>
 *   <bo><tox>tom</tox></bo>
 *   <mer><fo1>1</fo1><fo1>2</fo1></mer>
 *   <collection><member>1</member><member>2</member></collection>
 * <!-- ^-from 'foo' ^-from? -->
 *   <huk foo="fufu"/>
 * </foo>
 */
namespace sergut {

class XmlDeserializerTiny : public DeserializerBase
{
public:
  class ErrorContext:  public ParsingException::ErrorContext {
  public:
    ErrorContext(const TiXmlNode& pNode) : node(&pNode) { }
    std::size_t getRow() const override;
    std::size_t getColumn() const override;
  private:
    const TiXmlNode* node = nullptr;
  };

public:
  XmlDeserializerTiny(const std::string& xml);
  XmlDeserializerTiny(const XmlDeserializerTiny& ref);
  ~XmlDeserializerTiny();


  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, XmlDeserializerTiny&>::type
  operator&(const NamedMemberForDeserialization<T>& data) {
    return extractSimpleType(data);
  }

  // String types
  XmlDeserializerTiny& operator&(const NamedMemberForDeserialization<std::string>& data) {
    return extractSimpleType(data);
  }


  XmlDeserializerTiny& operator&(const NamedMemberForDeserialization<const char*>& data) = delete;

  // Containers as members
  template<typename CDT>
  XmlDeserializerTiny& operator&(const NamedMemberForDeserialization<std::vector<CDT>>& data) {
    assert(valueType == ValueType::Child);
    while(currentElement->FirstChildElement(data.name) != nullptr) {
      CDT tmp;
      operator&(NamedMemberForDeserialization<CDT>(data.name, tmp, true));
      data.data.push_back(tmp);
    }
    return *this;
  }

  template<typename CDT>
  XmlDeserializerTiny& operator&(const NamedMemberForDeserialization<std::list<CDT>>& data) {
    assert(valueType == ValueType::Child);
    while(currentElement->FirstChildElement(data.name) != nullptr) {
      CDT tmp;
      operator&(NamedMemberForDeserialization<CDT>(data.name, tmp, true));
      data.data.push_back(tmp);
    }
    return *this;
  }

  template<typename CDT>
  XmlDeserializerTiny& operator&(const NamedMemberForDeserialization<std::set<CDT>>& data) {
    assert(valueType == ValueType::Child);
    while(currentElement->FirstChildElement(data.name) != nullptr) {
      CDT tmp;
      operator&(NamedMemberForDeserialization<CDT>(data.name, tmp, true));
      data.data.insert(tmp);
    }
    return *this;
  }

  // Members that can be converted to string
  template<typename DT>
  auto operator&(const NamedMemberForDeserialization<DT>& data)
  -> decltype(deserializeFromString(data.data, std::string()),*this)
  {
    deserializeFromString(data.data, popString(data.name, data.mandatory));
    return *this;
  }

  template<typename DT>
  auto operator&(const NamedMemberForDeserialization<DT>& data)
  -> decltype(serialize(DummySerializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)),*this)
  {
    // retrieve the correct node (with exception of the root tag)
    TiXmlElement* el = currentElement->ToElement();
    if(xmlDocument == nullptr) {
      el = currentElement->FirstChildElement(data.name);
      if(el == nullptr) {
        if(data.mandatory) {
          throw ParsingException("Misssing mandatory child element", ErrorContext(*currentElement));
        }
        return *this;
      }
      currentElement = el;
    }

    {
      // deserialize the children
      XmlDeserializerTiny ser(*this);
      serialize(ser, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
      assert(currentElement == el);
    }

    // clean up (with exception of the root tag)
    if(xmlDocument == nullptr) {
      currentElement = el->Parent()->ToElement();
      currentElement->RemoveChild(el);
    }
    return *this;
  }

  /// Members until this marker are rendered as XML-Attributes, after it as sub-elements
  XmlDeserializerTiny& operator&(const ChildrenFollow&);

  /// Members until this marker are rendered as XML-Attributes,
  /// After this marker there should only be one member left, that must be
  /// renderable as a simple XML-Type (i.e. a number or a string)
  XmlDeserializerTiny& operator&(const PlainChildFollows&);

  /// Initial call to the serializer
  /// \param name The name of the outer tag
  template<typename DT>
  DT deserializeData(const char* name, const sergut::ValueType pValueType) {
    if(name != nullptr && ::strcmp(currentElement->Value(), name) != 0) {
      throw ParsingException("Wrong root tag", ErrorContext(*currentElement));
    }
    DT data;
    valueType = pValueType;
    *this & toNamedMember(name, data, true);
    return data;
  }

  /// Initial call to the serializer
  /// \param name The name of the outer tag
  template<typename DT>
  DT deserializeNestedData(const char* outerName, const char* innerName, const sergut::ValueType pValueType) {
    if(outerName != nullptr && std::strcmp(currentElement->Value(), outerName) != 0) {
      throw ParsingException("Wrong root tag", ErrorContext(*currentElement));
    }
    DT data;
    valueType = pValueType;
    *this & toNamedMember(outerName, toNamedMember(innerName, data, true), true);
    return data;
  }

private:
  std::string popString(const char* name, const bool mandatory) {
    std::string d;
    *this & toNamedMember(name, d, mandatory);
    return d;
  }

  template<typename DT>
  XmlDeserializerTiny& extractSimpleType(const NamedMemberForDeserialization<DT>& data) {
    switch(valueType) {
    case ValueType::Attribute:
      extractAttribute(data);
    break;
    case ValueType::Child:
      extractSimpleChild(data);
      break;
    case ValueType::SingleChild:
      extractSingleChild(data);
      break;
    }
    return *this;
  }

  template<typename DT>
  void doReadInto(const char* str, DT& data) {
    std::istringstream(str) >> data;
  }

  void doReadInto(const char* str, std::string& data);
  void doReadInto(const char* str, unsigned char& data);

  template<typename DT>
  void readInto(const char* str, const NamedMemberForDeserialization<DT>& data, const ParsingException::ErrorContext& errorContext) {
    if(str == nullptr) {
      if(data.mandatory) {
        throw ParsingException("missing mandatory attribute '" + std::string(data.name) + "'", errorContext);
      }
      return;
    }
    doReadInto(str, data.data);
  }

  void readInto(const char* str, const NamedMemberForDeserialization<char>& data, const ParsingException::ErrorContext& errorContext) {
    if(str == nullptr || str[0] == '\0') {
      if(data.mandatory) {
        throw ParsingException("Missing mandatory attribute '" + std::string(data.name) + "'", errorContext);
      }
      return;
    }
    data.data = str[0];
  }

  template<typename DT>
  void extractAttribute(const NamedMemberForDeserialization<DT>& data) {
    TiXmlElement* e = currentElement->ToElement();
    assert(e != nullptr);
    const char* a = e->Attribute(data.name);
    readInto(a, data, ErrorContext(*e));
    e->RemoveAttribute(data.name);
  }

  template<typename DT>
  void extractSimpleChild(const NamedMemberForDeserialization<DT>& data) {
    TiXmlElement* e = currentElement->FirstChildElement(data.name);
    TiXmlNode*    n = (e == nullptr) ? nullptr : e->FirstChild();
    TiXmlText*    t = (n == nullptr) ? nullptr : n->ToText();
    readInto((t == nullptr) ? nullptr : t->Value(), data, t != nullptr ? ErrorContext(*t) : ErrorContext(*currentElement));
    currentElement->RemoveChild(e);
  }

  template<typename DT>
  void extractSingleChild(const NamedMemberForDeserialization<DT>& data) {
    TiXmlNode* n = currentElement->FirstChild();
    TiXmlText* t = (n == nullptr) ? nullptr : n->ToText();
    readInto((t == nullptr) ? nullptr : t->Value(), data, t != nullptr ? ErrorContext(*t) : ErrorContext(*currentElement));
    currentElement->RemoveChild(t);
  }

private:
  const ValueType parentValueType;
  ValueType valueType;
  std::unique_ptr<TiXmlDocument> xmlDocument;
  TiXmlElement* currentElement;
};

} // namespace sergut
