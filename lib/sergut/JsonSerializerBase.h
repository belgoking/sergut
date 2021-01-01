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

#include <list>
#include <set>
#include <sstream>
#include <vector>

namespace sergut {
namespace detail {

class JsonSerializerBase: public SerializerBase
{
protected:
  struct Impl;
  struct LevelStatus;
public:
  enum class Flags {
    None = 0,
    BoolAsInt = 1  // set by default for backward-compatibility
  };
  JsonSerializerBase(const Flags flags);
  JsonSerializerBase(const JsonSerializerBase& ref);
  ~JsonSerializerBase();

  std::string str() const;

protected:
  void writeEscaped(const std::string& str);
  void addCommaIfNeeded();
  std::ostream& out();
  bool hasFlag(Flags flag) const;

private:
  Impl* impl;
};

} // namespace detail

template<typename Dialect>
class JsonSerializerBase: public detail::JsonSerializerBase
{
  struct Impl;
  struct LevelStatus;
public:
  JsonSerializerBase(const Flags flags = Flags::BoolAsInt)
    : detail::JsonSerializerBase(flags)
  { }
  JsonSerializerBase(const JsonSerializerBase& ref) = default;

  template<typename DT>
  Dialect& operator&(const NamedMemberForSerialization<DT>& data) {
    addCommaIfNeeded();
    if(data.name) { out() << "\"" << data.name << "\":"; }
    static_cast<Dialect*>(this)->serializeValue(data.data);
    return static_cast<Dialect&>(*this);
  }

  void serializeValue(const long long data) { out() << data; }
  void serializeValue(const long data) { out() << data; }
  void serializeValue(const int data) { out() << data; }
  void serializeValue(const short data) { out() << data; }
  void serializeValue(const signed char data) { out() << static_cast<short>(data); }
  void serializeValue(const unsigned long long data) { out() << data; }
  void serializeValue(const unsigned long data) { out() << data; }
  void serializeValue(const unsigned int data) { out() << data; }
  void serializeValue(const unsigned short data) { out() << data; }
  void serializeValue(const unsigned char data) { out() << static_cast<unsigned short>(data); }
  void serializeValue(const double data) { out() << data; }
  void serializeValue(const float data) { out() << data; }
  void serializeValue(const bool data) {
    if(hasFlag(Flags::BoolAsInt)) {
      out() << (data ? 1 : 0);
    } else {
      out() << (data ? "true" : "false");
    }
  }



  void serializeValue(const std::string& data) {
    out() << "\"";
    writeEscaped(data);
    out() << "\"";
  }
  void serializeValue(const char data[]) { serializeValue(std::string(data)); }
  void serializeValue(const char*& data) { serializeValue(std::string(data)); }
  void serializeValue(const char data) { serializeValue(std::string(1, data)); }

  // Containers as members
  template<typename DT>
  void serializeCollection(const DT& data) {
    out() << "[";
    bool first=true;
    for(auto&& value: data) {
      if(!first) {
        out() << ",";
      } else {
        first=false;
      }
      Dialect ser(*this);
      ser.serializeValue(value);
    }
    out() << "]";
  }

  template<typename ValueType>
  void serializeValue(const std::vector<ValueType>& data) {
    return serializeCollection(data);
  }

  template<typename ValueType>
  void serializeValue(const std::list<ValueType>& data) {
    return serializeCollection(data);
  }

  template<typename ValueType>
  void serializeValue(const std::set<ValueType>& data) {
    return serializeCollection(data);
  }

  template<typename DT>
  auto serializeValue(const DT& data)
  -> decltype(serialize(declval<Dialect&>(), data, static_cast<typename std::decay<DT>::type*>(nullptr)), void())
  {
    out() << "{";
    Dialect ser(static_cast<Dialect&>(*this));
    serialize(ser, data, static_cast<typename std::decay<DT>::type*>(nullptr));
    out() << "}";
  }

  // Members that can be converted to string
  template<typename DT>
  auto serializeValue(const DT& data)
  -> decltype(serializeToString(data), void())
  {
    serializeValue(serializeToString(data));
  }

  // This is unused for JSON
  Dialect& operator&(const ChildrenFollow&) { return static_cast<Dialect&>(*this); }

  // This is unused for JSON
  Dialect& operator&(const PlainChildFollows&) { return static_cast<Dialect&>(*this); }

  /// \param name this is not needed for JSON, we just add it here for symmetry to XML.
  template<typename DT>
  void serializeData(const DT& data) {
    static_cast<Dialect*>(this)->serializeValue(data);
  }
};

} // namespace sergut
