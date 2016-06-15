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

#include "sergut/SerializerBase.h"
#include "sergut/Util.h"

#include <cassert>
#include <list>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
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

class XmlSerializer : public SerializerBase
{
  class Impl;
  class LevelStatus;
  enum class ValueType { Attribute, Child, SingleChild };
public:
  XmlSerializer();
  XmlSerializer(const XmlSerializer& ref);
  ~XmlSerializer();

  // Signed integers
  XmlSerializer& operator&(const NamedMemberForSerialization<long long>& data) {
    return writeSimpleType(data);
  }
  XmlSerializer& operator&(const NamedMemberForSerialization<long>& data) {
    return writeSimpleType(data);
  }
  XmlSerializer& operator&(const NamedMemberForSerialization<int>& data) {
    return writeSimpleType(data);
  }
  XmlSerializer& operator&(const NamedMemberForSerialization<short>& data) {
    return writeSimpleType(data);
  }

  // Unsigned integers
  XmlSerializer& operator&(const NamedMemberForSerialization<unsigned long long>& data) {
    return writeSimpleType(data);
  }
  XmlSerializer& operator&(const NamedMemberForSerialization<unsigned long>& data) {
    return writeSimpleType(data);
  }
  XmlSerializer& operator&(const NamedMemberForSerialization<unsigned int>& data) {
    return writeSimpleType(data);
  }
  XmlSerializer& operator&(const NamedMemberForSerialization<unsigned short>& data) {
    return writeSimpleType(data);
  }
  XmlSerializer& operator&(const NamedMemberForSerialization<unsigned char>& data) {
    return writeSimpleType(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }

  XmlSerializer& operator&(const NamedMemberForSerialization<double>& data) {
    return writeSimpleType(data);
  }
  XmlSerializer& operator&(const NamedMemberForSerialization<float>& data) {
    return writeSimpleType(data);
  }

  // String types
  XmlSerializer& operator&(const NamedMemberForSerialization<std::string>& data) {
    return writeSimpleType(data);
  }

  XmlSerializer& operator&(const NamedMemberForSerialization<char>& data) {
    return operator&(toNamedMember(data.name, std::string(1, data.data), data.mandatory));
  }

  XmlSerializer& operator&(const NamedMemberForSerialization<const char*>& data) = delete;


  // Containers as members
  template<typename DT>
  XmlSerializer& serializeCollection(const NamedMemberForSerialization<DT>& data) {
    if(getValueType() != ValueType::Child) {
      throw std::logic_error(std::string("Wrong ValueType to serialize '")+ data.name + "' get your code right!");
    }
    for(auto&& value: data.data) {
      *this & toNamedMember(data.name, value, true);
    }
    return *this;
  }


  template<typename ValueType>
  XmlSerializer& operator&(const NamedMemberForSerialization<std::vector<ValueType>>& data) {
    return serializeCollection(data);
  }


  template<typename ValueType>
  XmlSerializer& operator&(const NamedMemberForSerialization<std::list<ValueType>>& data) {
    return serializeCollection(data);
  }


  template<typename ValueType>
  XmlSerializer& operator&(const NamedMemberForSerialization<std::set<ValueType>>& data) {
    return serializeCollection(data);
  }


  // Members that can be converted to string
  template<typename DT>
  auto operator&(const NamedMemberForSerialization<DT>& data)
  -> decltype(serializeToString(data.data),*this)
  {
    return operator&(toNamedMember(data.name, serializeToString(data.data), data.mandatory));
  }

  template<typename DT>
  auto operator&(const NamedMemberForSerialization<DT>& data)
  -> decltype(serialize(DummySerializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)),*this)
  {
    assert(getValueType()==ValueType::Child);

    {
      XmlSerializer ser(*this);
      // Render opening tag
      out() << "<" << data.name;

      // Render all attributes and children
      serialize(ser, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));

      // Render closing tag
      switch(getValueType()) {
      case ValueType::Attribute:
        out() << "/>";
        break;
      case ValueType::Child:
      case ValueType::SingleChild:
        out() << "</" << data.name << ">";
        break;
      }
    }
    return *this;
  }

  /// Members until this marker are rendered as XML-Attributes, after it as sub-elements
  XmlSerializer& operator&(const ChildrenFollow&);

  /// Members until this marker are rendered as XML-Attributes,
  /// After this marker there should only be one member left, that must be
  /// renderable as a simple XML-Type (i.e. a number or a string)
  XmlSerializer& operator&(const PlainChildFollows&);

  /// Initial call to the serializer
  /// \param name The name of the outer tag
  template<typename DT>
  void serializeData(const char* name, const DT& data) {
    *this & toNamedMember(name, data, true);
  }

  std::string str() const;

private:
  template<typename DT>
  XmlSerializer& writeSimpleType(const NamedMemberForSerialization<DT>& data) {
    switch(getValueType()) {
    case ValueType::Attribute:
      writeAttribute(data);
    break;
    case ValueType::Child:
      writeSimpleChild(data);
      break;
    case ValueType::SingleChild:
      writeEscaped(data.data);
      break;
    }
    return *this;
  }
  template<typename DT>
  void writeAttribute(const NamedMemberForSerialization<DT>& data) {
    out() << " " << data.name << "=\"";
    writeEscaped(data.data);
    out() << "\"";
  }
  template<typename DT>
  void writeSimpleChild(const NamedMemberForSerialization<DT>& data) {
    out() << "<" << data.name << ">";
    writeEscaped(data.data);
    out() << "</" << data.name << ">";
  }

  template<typename DT>
  void writeEscaped(const DT& data) {
    out() << data;
  }

  void writeEscaped(const std::string& str);

  ValueType getValueType() const;
  std::ostringstream& out();

private:
  Impl* impl = nullptr;
};

} // namespace sergut
