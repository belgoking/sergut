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
#include "sergut/detail/DummySerializer.h"

#include <sstream>
#include <list>
#include <set>
#include <vector>

namespace sergut {

class JsonSerializer: public SerializerBase
{
  class Impl;
  class LevelStatus;
public:
  JsonSerializer();
  JsonSerializer(const JsonSerializer& ref);
  ~JsonSerializer();

  JsonSerializer& operator&(const NamedMemberForSerialization<long long>& data) {
    addCommaIfNeeded();
    if(data.name) { out() << "\"" << data.name << "\":"; }
    out() << data.data;
    return *this;
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<long>& data) {
    return operator&(toNamedMember(data.name, static_cast<long long>(data.data), data.mandatory));
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<int>& data) {
    return operator&(toNamedMember(data.name, static_cast<long long>(data.data), data.mandatory));
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<short>& data) {
    return operator&(toNamedMember(data.name, static_cast<long long>(data.data), data.mandatory));
  }

  JsonSerializer& operator&(const NamedMemberForSerialization<unsigned long long>& data) {
    addCommaIfNeeded();
    if(data.name) { out() << "\"" << data.name << "\":"; }
    out() << data.data;
    return *this;
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<unsigned long>& data) {
    return operator&(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<unsigned int>& data) {
    return operator&(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<unsigned short>& data) {
    return operator&(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<unsigned char>& data) {
    return operator&(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }

  JsonSerializer& operator&(const NamedMemberForSerialization<double>& data) {
    addCommaIfNeeded();
    if(data.name) { out() << "\"" << data.name << "\":"; }
    out() << data.data;
    return *this;
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<float>& data) {
    return operator&(toNamedMember(data.name, static_cast<double>(data.data), data.mandatory));
  }

  // ToDo: Implement sensible escaping
  static std::string escape(const std::string& str) { return str; }

  JsonSerializer& operator&(const NamedMemberForSerialization<std::string>& data) {
    addCommaIfNeeded();
    if(data.name) { out() << "\"" << data.name << "\":"; }
    out() << "\"";
    writeEscaped(data.data);
    out() << "\"";
    return *this;
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<const char*>& data) {
    return operator&(toNamedMember(data.name, std::string(data.data), data.mandatory));
  }
  JsonSerializer& operator&(const NamedMemberForSerialization<char>& data) {
    return operator&(toNamedMember(data.name, std::string(1, data.data), data.mandatory));
  }


  // Containers as members
  template<typename DT>
  JsonSerializer& serializeCollection(const NamedMemberForSerialization<DT>& data) {
    if(data.mandatory || !data.data.empty()) {
      addCommaIfNeeded();
      if(data.name) { out() << "\"" << data.name << "\":"; }
      out() << "[";
      bool first=true;
      for(auto&& value: data.data) {
        if(!first) {
          out() << ",";
        } else {
          first=false;
        }
        JsonSerializer ser(*this);
        *this & toNamedMember(nullptr, value, true);
      }
      out() << "]";
    }
    return *this;
  }

  template<typename ValueType>
  JsonSerializer& operator&(const NamedMemberForSerialization<std::vector<ValueType>>& data) {
    return serializeCollection(data);
  }

  template<typename ValueType>
  JsonSerializer& operator&(const NamedMemberForSerialization<std::list<ValueType>>& data) {
    return serializeCollection(data);
  }

  template<typename ValueType>
  JsonSerializer& operator&(const NamedMemberForSerialization<std::set<ValueType>>& data) {
    return serializeCollection(data);
  }

  template<typename DT>
  auto operator&(const NamedMemberForSerialization<DT>& data)
  -> decltype(serialize(detail::DummySerializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)), *this)
  {
    addCommaIfNeeded();
    {
      if(data.name) { out() << "\"" << data.name << "\":"; }
      out() << "{";
      JsonSerializer ser(*this);
      serialize(ser, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
      out() << "}";
    }
    return *this;
  }

  // Members that can be converted to string
  template<typename DT>
  auto operator&(const NamedMemberForSerialization<DT>& data)
  -> decltype(serializeToString(data.data), *this)
  {
    return operator&(toNamedMember(data.name, serializeToString(data.data), data.mandatory));
  }

  // This is unused for JSON
  JsonSerializer& operator&(const ChildrenFollow&) { return *this; }

  // This is unused for JSON
  JsonSerializer& operator&(const PlainChildFollows&) { return *this; }

  /// \param name this is not needed for JSON, we just add it here for symmetry to XML.
  template<typename DT>
  void serializeData(const std::string& /*name*/, const DT& data) {
    out() << "{";
    serialize(*this, data, static_cast<typename std::decay<DT>::type*>(nullptr));
    out() << "}";
  }

  std::string str() const;

private:
  void writeEscaped(const std::string& str);
  void addCommaIfNeeded();
  std::ostringstream& out();

private:
  Impl* impl;
};

} // namespace sergut
