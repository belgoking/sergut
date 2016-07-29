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

#include "sergut/detail/Member.h"
#include "sergut/detail/TypeName.h"
#include "sergut/SerializerBase.h"
#include "sergut/Util.h"
#include "sergut/XmlValueType.h"

#include <sstream>
#include <limits>
#include <list>
#include <memory>
#include <set>
#include <vector>

namespace sergut {
namespace detail {

class JavaClassGeneratorBase: public SerializerBase
{
public:
  virtual ~JavaClassGeneratorBase() { }

  std::string getPath() const;

public: // the following is not meant to be a public interface, it is needed for serialization
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<long long>& data) {
    return addMember(toJavaIntType(std::numeric_limits<long long>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<unsigned long long>& data) {
    return addMember(toJavaIntType(std::numeric_limits<unsigned long long>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<long>& data) {
    return addMember(toJavaIntType(std::numeric_limits<long>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<unsigned long>& data) {
    return addMember(toJavaIntType(std::numeric_limits<unsigned long>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<int>& data) {
    return addMember(toJavaIntType(std::numeric_limits<int>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<unsigned int>& data) {
    return addMember(toJavaIntType(std::numeric_limits<unsigned int>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<short>& data) {
    return addMember(toJavaIntType(std::numeric_limits<short>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<unsigned short>& data) {
    return addMember(toJavaIntType(std::numeric_limits<unsigned short>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<signed char>& data) {
    return addMember(toJavaIntType(std::numeric_limits<char>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<unsigned char>& data) {
    return addMember(toJavaIntType(std::numeric_limits<unsigned char>::max()), data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<double>& data) {
    return addMember("double", data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<float>& data) {
    return addMember("float", data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<char>& data) {
    return addMember("char", data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<bool>& data) {
    return addMember("boolean", data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<std::string>& data) {
    return addMember("java::lang::String", data.name);
  }
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<const char*>& data) {
    return addMember("java::lang::String", data.name);
  }

  // Containers as members
  template<typename DT>
  JavaClassGeneratorBase& serializeCollection(const NamedMemberForSerialization<DT>& data) {
    addArrayMember(std::string(getTypeName(static_cast<DT*>(nullptr))), data.name);
    return *this;
  }

  template<typename ValueType>
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<std::vector<ValueType>>& data) {
    ValueType vt;
    return serializeCollection(toNamedMember(data.name, vt, true));
  }

  template<typename ValueType>
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<std::list<ValueType>>& data) {
    ValueType vt;
    return serializeCollection(toNamedMember(data.name, vt, true));
  }

  template<typename ValueType>
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<std::set<ValueType>>& data) {
    ValueType vt;
    return serializeCollection(toNamedMember(data.name, vt, true));
  }

  template<typename DT>
  JavaClassGeneratorBase& operator&(const NamedMemberForSerialization<DT>& data)
  {
    addMember(std::string(getTypeName(&data.data)), data.name);
    return *this;
  }

  JavaClassGeneratorBase& operator&(const ChildrenFollow&) {
    xmlValueType = sergut::XmlValueType::Child;
    return *this;
  }

  JavaClassGeneratorBase& operator&(const PlainChildFollows&) {
    xmlValueType = sergut::XmlValueType::Child;
    return *this;
  }

protected:
  virtual detail::TypeName getTypeMapping(const std::string& cppTypeName, const detail::TypeName::CollectionType collectionType) const {
    return detail::TypeName(cppTypeName, collectionType);
  }

private:
  JavaClassGeneratorBase& addMember(const std::string& typeName, const std::string& memberName) {
    members.push_back(detail::Member(memberName, getTypeMapping(typeName, detail::TypeName::CollectionType::None), xmlValueType));
    return *this;
  }

  JavaClassGeneratorBase& addArrayMember(const std::string& typeName, const std::string& memberName) {
    members.push_back(detail::Member(memberName, getTypeMapping(typeName, detail::TypeName::CollectionType::Array), xmlValueType));
    return *this;
  }

  std::string str() const;

private:
  friend std::ostream& operator<<(std::ostream& ostr, const JavaClassGeneratorBase& cls);

  const char* toJavaIntType(const uint64_t maxVal);

protected:
  detail::TypeName            typeName;
  std::vector<detail::Member> members;
  sergut::XmlValueType        xmlValueType = sergut::XmlValueType::Attribute;
};

std::ostream& operator<<(std::ostream& ostr, const JavaClassGeneratorBase& cls);

} // namespace detail
} // namespace sergut
