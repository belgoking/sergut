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
#include "sergut/Util.h"
#include "sergut/XmlValueType.h"
#include "sergut/detail/DummySerializer.h"
#include "sergut/detail/XmlDeserializerHelper.h"
#include "sergut/misc/ReadHelper.h"

#include <cassert>
#include <cstring>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <sstream>
#include <vector>

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
namespace detail {

template<typename DT, typename XmlDomParser>
void extractAndDeleteAttribute(const NamedMemberForDeserialization<DT>& data, const XmlDomParser& currentElement) {
  This_function_needs_to_be_implemented_by_specific_DOM_implementation(data, currentElement);
}

template<typename DT, typename XmlDomParser>
void extractAndDeleteSimpleChild(const NamedMemberForDeserialization<DT>& data, const XmlDomParser& currentElement) {
  This_function_needs_to_be_implemented_by_specific_DOM_implementation(data, currentElement);
}

template<typename DT, typename XmlDomParser>
void extractAndDeleteSingleChild(const NamedMemberForDeserialization<DT>& data, const XmlDomParser& currentElement) {
  This_function_needs_to_be_implemented_by_specific_DOM_implementation(data, currentElement);
}

template<typename DT>
inline
void readInto(const char* str, const NamedMemberForDeserialization<DT>& data,
              const ParsingException::ErrorContext& errorContext)
{
  if(str == nullptr) {
    if(data.mandatory) {
      throw ParsingException("missing mandatory attribute '" + std::string(data.name) + "'", errorContext);
    }
    return;
  }
  sergut::misc::ReadHelper::readInto(sergut::misc::ConstStringRef(str, str + std::strlen(str)), data.data);
}

template<typename XmlDomParser>
class XmlDeserializerDomBase : public DeserializerBase
{
public:
  class ErrorContext;

public:
  XmlDeserializerDomBase(const std::string& xml);
  XmlDeserializerDomBase(const XmlDeserializerDomBase& ref)
    : parentValueType(ref.valueType)
    , valueType(XmlValueType::Attribute)
    , currentElement(ref.currentElement)
  { }

  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, XmlDeserializerDomBase&>::type
  operator&(const NamedMemberForDeserialization<T>& data) {
    return extractSimpleType(data);
  }

  // String types
  XmlDeserializerDomBase& operator&(const NamedMemberForDeserialization<std::string>& data) {
    return extractSimpleType(data);
  }


  XmlDeserializerDomBase& operator&(const NamedMemberForDeserialization<const char*>& data) = delete;

  // Containers as members
  template<typename CDT>
  XmlDeserializerDomBase& operator&(const NamedMemberForDeserialization<std::vector<CDT>>& data) {
    assert(valueType == XmlValueType::Child);
    while(currentElement->FirstChildElement(data.name) != nullptr) {
      CDT tmp;
      operator&(NamedMemberForDeserialization<CDT>(data.name, tmp, true));
      data.data.push_back(tmp);
    }
    return *this;
  }

  template<typename CDT>
  XmlDeserializerDomBase& operator&(const NamedMemberForDeserialization<std::list<CDT>>& data) {
    assert(valueType == XmlValueType::Child);
    while(currentElement->FirstChildElement(data.name) != nullptr) {
      CDT tmp;
      operator&(NamedMemberForDeserialization<CDT>(data.name, tmp, true));
      data.data.push_back(tmp);
    }
    return *this;
  }

  template<typename CDT>
  XmlDeserializerDomBase& operator&(const NamedMemberForDeserialization<std::set<CDT>>& data) {
    assert(valueType == XmlValueType::Child);
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
  -> decltype(serialize(DummyDeserializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)),*this)
  {
    // retrieve the correct node (with exception of the root tag)
    auto* el = currentElement->ToElement();
    if(!xmlDocument) {
      el = currentElement->FirstChildElement(data.name);
      if(el == nullptr) {
        if(data.mandatory) {
          throw ParsingException("Missing mandatory child element", ErrorContext(*currentElement));
        }
        return *this;
      }
      currentElement = el;
    }

    {
      // deserialize the children
      XmlDeserializerDomBase ser(*this);
      serialize(ser, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
      assert(currentElement == el);
    }

    // clean up (with exception of the root tag)
    if(!xmlDocument) {
      currentElement = XmlDomParser::getParentNode(*el);
      XmlDomParser::deleteChildNode(*currentElement, *el);
    }
    return *this;
  }

  /// Members until this marker are rendered as XML-Attributes, after it as sub-elements
  XmlDeserializerDomBase& operator&(const ChildrenFollow&) {
    assert(valueType == XmlValueType::Attribute);
    valueType = XmlValueType::Child;
    return *this;
  }

  /// Members until this marker are rendered as XML-Attributes,
  /// After this marker there should only be one member left, that must be
  /// renderable as a simple XML-Type (i.e. a number or a string)
  XmlDeserializerDomBase& operator&(const PlainChildFollows&) {
    assert(valueType==XmlValueType::Attribute);
    valueType = XmlValueType::SingleChild;
    return *this;
  }

  /// Initial call to the serializer
  /// \param name The name of the outer tag
  template<typename DT>
  DT deserializeData(const char* name) {
    if(name != nullptr && std::strcmp(currentElement->Value(), name) != 0) {
      throw ParsingException("Wrong root tag", ErrorContext(*currentElement));
    }
    DT data;
    valueType = XmlValueType::Attribute;
    if(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<DT>()) {
      *this & toNamedMember(name, toNestedMember("DUMMY", data, true, XmlValueType::SingleChild), true);
    } else {
      *this & toNamedMember(name, data, true);
    }
    return data;
  }

  /// Initial call to the serializer
  /// \param name The name of the outer tag
  template<typename DT, XmlValueType xmlValueType = XmlValueType::Child>
  DT deserializeNestedData(const char* outerName, const char* innerName)
  {
    static_assert(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<DT>()
                  || xmlValueType == XmlValueType::Child,
                  "Datatypes that cannot be serialized as an Attribute (i.e. those for which serialize() is called), "
                  "must be deserialized with xmlValueType == sergut::XmlValueType::Child.");
    if(outerName != nullptr && std::strcmp(currentElement->Value(), outerName) != 0) {
      throw ParsingException("Wrong root tag", ErrorContext(*currentElement));
    }
    DT data;
    valueType = XmlValueType::Child;
    *this & toNamedMember(outerName, toNestedMember(innerName, data, true, xmlValueType), true);
    return data;
  }

private:
  std::string popString(const char* name, const bool mandatory) {
    std::string d;
    *this & toNamedMember(name, d, mandatory);
    return d;
  }

  template<typename DT>
  XmlDeserializerDomBase& extractSimpleType(const NamedMemberForDeserialization<DT>& data) {
    switch(valueType) {
    case XmlValueType::Attribute:
      XmlDomParser::extractAndDeleteAttribute(data, *currentElement);
    break;
    case XmlValueType::Child:
      XmlDomParser::extractAndDeleteSimpleChild(data, *currentElement);
      break;
    case XmlValueType::SingleChild:
      XmlDomParser::extractAndDeleteSingleChild(data, *currentElement);
      break;
    }
    return *this;
  }

private:
  const XmlValueType parentValueType;
  XmlValueType valueType;
  std::unique_ptr<typename XmlDomParser::Document> xmlDocument;
  typename XmlDomParser::Node* currentElement;
};

} // namespace detail
} // namespace sergut
