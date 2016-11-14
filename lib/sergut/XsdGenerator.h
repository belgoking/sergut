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

#include "sergut/SerializationException.h"
#include "sergut/SerializerBase.h"
#include "sergut/Util.h"
#include "sergut/XmlValueType.h"
#include "sergut/detail/DummySerializer.h"
#include "sergut/detail/XmlDeserializerHelper.h"
#include "sergut/misc/ConstStringRef.h"

#include <cassert>
#include <list>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

namespace sergut {

class XsdGenerator: public SerializerBase
{
  class SingleChildFinder: public SerializerBase {
  public:
    SingleChildFinder& operator&(const NamedMemberForSerialization<long long>&) {
      return handleSimpleType("long long");
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<long>&) {
      return handleSimpleType("long");
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<int>&) {
      return handleSimpleType("int");
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<short>&) {
      return handleSimpleType("short");
    }

    SingleChildFinder& operator&(const NamedMemberForSerialization<unsigned long long>&) {
      return handleSimpleType("ull");
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<unsigned long>&) {
      return handleSimpleType("ul");
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<unsigned int>&) {
      return handleSimpleType("uint");
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<unsigned short>&) {
      return handleSimpleType("ushort");
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<unsigned char>&) {
      return handleSimpleType("uchar");
    }

    SingleChildFinder& operator&(const NamedMemberForSerialization<double>&) {
      return handleSimpleType("double");
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<float>&) {
      return handleSimpleType("float");
    }

    SingleChildFinder& operator&(const NamedMemberForSerialization<std::string>&) {
      return handleSimpleType("string");
      return *this;
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<const char*>&) {
      return handleSimpleType("string");
      return *this;
    }
    SingleChildFinder& operator&(const NamedMemberForSerialization<char>&) {
      return handleSimpleType("char");
    }
    template<typename ValueType>
    SingleChildFinder& operator&(const NamedMemberForSerialization<std::vector<ValueType>>&) {
      return *this;
    }

    template<typename ValueType>
    SingleChildFinder& operator&(const NamedMemberForSerialization<std::list<ValueType>>&) {
      return *this;
    }

    template<typename ValueType>
    SingleChildFinder& operator&(const NamedMemberForSerialization<std::set<ValueType>>&) {
      return *this;
    }

    template<typename DT>
    auto operator&(const NamedMemberForSerialization<DT>& data)
    -> decltype(serialize(detail::DummySerializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)), *this)
    {
      return *this;
    }

    // Members that can be converted to string
    template<typename DT>
    auto operator&(const NamedMemberForSerialization<DT>& data)
    -> decltype(serializeToString(data.data), *this)
    {
      return handleSimpleType("string");
      return *this;
    }
    SingleChildFinder& operator&(const ChildrenFollow&) {
      _memberXmlValueType = XmlValueType::Child;
      return *this;
    }

    // This is unused for URL encoding
    SingleChildFinder& operator&(const PlainChildFollows&) {
      _memberXmlValueType = XmlValueType::SingleChild;
      return *this;
    }

    bool hasSingleChild() const { return _type != nullptr; }
    const char* getSingleChildType() const { return _type; }
  private:
    SingleChildFinder& handleSimpleType(const char* type) {
      if(_memberXmlValueType == XmlValueType::SingleChild) {
        _type = type;
      }
      return *this;
    }

  private:
    const char* _type = nullptr;
    XmlValueType _memberXmlValueType = XmlValueType::Attribute;
  };


public:
  XsdGenerator(std::ostream& out) : _out(out) { }
  ~XsdGenerator() { }

  XsdGenerator& operator&(const NamedMemberForSerialization<long long>& data) {
    return writeSimpleType(data.name, "long long");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<long>& data) {
    return writeSimpleType(data.name, "long");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<int>& data) {
    return writeSimpleType(data.name, "int");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<short>& data) {
    return writeSimpleType(data.name, "short");
  }

  XsdGenerator& operator&(const NamedMemberForSerialization<unsigned long long>& data) {
    return writeSimpleType(data.name, "ull");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<unsigned long>& data) {
    return writeSimpleType(data.name, "ul");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<unsigned int>& data) {
    return writeSimpleType(data.name, "uint");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<unsigned short>& data) {
    return writeSimpleType(data.name, "ushort");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<unsigned char>& data) {
    return writeSimpleType(data.name, "uchar");
  }

  XsdGenerator& operator&(const NamedMemberForSerialization<double>& data) {
    return writeSimpleType(data.name, "double");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<float>& data) {
    return writeSimpleType(data.name, "float");
  }

  XsdGenerator& operator&(const NamedMemberForSerialization<std::string>& data) {
    return writeSimpleType(data.name, "string");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<const char*>& data) {
    return writeSimpleType(data.name, "string");
  }
  XsdGenerator& operator&(const NamedMemberForSerialization<char>& data) {
    return writeSimpleType(data.name, "char");
  }

  // Containers as members
  template<typename DT>
  XsdGenerator& serializeCollection(const NamedMemberForSerialization<DT>& collection) {
    _out << "<xs:complextype>\n"
            "<xs:sequence>\n";
    const typename DT::value_type collectionElement;
    *this & toNamedMember(collection.name, collectionElement, collection.mandatory);
    _out << "</xs:sequence>\n"
            "</xs:complextype>\n";
    return *this;
  }

  template<typename ValueType>
  XsdGenerator& operator&(const NamedMemberForSerialization<std::vector<ValueType>>& data) {
    return serializeCollection(data);
  }
  template<typename ValueType>
  XsdGenerator& operator&(const NamedMemberForSerialization<std::list<ValueType>>& data) {
    return serializeCollection(data);
  }
  template<typename ValueType>
  XsdGenerator& operator&(const NamedMemberForSerialization<std::set<ValueType>>& data) {
    return serializeCollection(data);
  }

  template<typename DT>
  auto operator&(const NamedMemberForSerialization<DT>& data)
  -> decltype(serialize(detail::DummySerializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)), *this)
  {
    _out << "<xs:complextype name='" << data.name;
    XsdGenerator::SingleChildFinder singleChildFinder;
    serialize(singleChildFinder, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
    if(singleChildFinder.hasSingleChild()) {
      _out << "' type='" << singleChildFinder.getSingleChildType() << "'>\n";
    } else {
      _out << "'>\n";
    }
    XsdGenerator ser(_out);
    serialize(ser, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
    _out << "</xs:complextype>\n";
    return *this;
  }

  // Members that can be converted to string
  template<typename DT>
  auto operator&(const NamedMemberForSerialization<DT>& data)
  -> decltype(serializeToString(data.data), *this)
  {
    return writeSimpleType(data.name, "string");
  }

  XsdGenerator& operator&(const ChildrenFollow&) {
    _memberXmlValueType = XmlValueType::Child;
    return *this;
  }
  XsdGenerator& operator&(const PlainChildFollows&) {
    _memberXmlValueType = XmlValueType::SingleChild;
    return *this;
  }

  template<typename DT>
  void serializeData(const std::string& name) {
    DT data;
    if(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<DT>()) {
      // In this case we don't call any serialize()-function. Thus this has
      // to be serialized to something like '<name>value</name>'.
      _memberXmlValueType = XmlValueType::Child;
    } else {
      // In this case the serialize()-function is called and we don't have to
      // do any special handling.
      _memberXmlValueType = XmlValueType::Attribute;
    }

    *this & toNamedMember(name.c_str(), data, true);
  }

  template<typename DT>
  void serializeNestedData(const std::string& outerName, const std::string& innerName, XmlValueType xmlValueType) {
    // Datatypes that cannot be serialized as an Attribute (i.e. those for which serialize() is called),
    // must be deserialized with xmlValueType == sergut::XmlValueType::Child.
    assert(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<DT>()
           || xmlValueType == XmlValueType::Child);
    _memberXmlValueType = xmlValueType;
    DT dummy;
    *this & toNamedMember(outerName.c_str(),
                          toNestedMember(innerName.c_str(), dummy, true, xmlValueType),
                          true);
  }

private:
  XsdGenerator& writeSimpleType(const char* name, const char* type) {
    switch(_memberXmlValueType) {
    case XmlValueType::Attribute:
      _out << "<xs:attribute name='" << name << "' type='" << type << "'/>\n";
      return *this;
    case XmlValueType::Child:
      _out << "<xs:element name='" << name << "' type='" << type << "'/>\n";
      return *this;
    case XmlValueType::SingleChild:
      // single childs are handled separatedly
      return *this;
    }
    assert(false);
    return *this;
  }

private:
  std::ostream& _out;
  XmlValueType _memberXmlValueType = XmlValueType::Attribute;
};

} // namespace sergut
