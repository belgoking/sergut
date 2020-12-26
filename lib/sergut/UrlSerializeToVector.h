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
#include "sergut/UrlNameCombiner.h"
#include "sergut/Util.h"
#include "sergut/misc/ConstStringRef.h"

#include <list>
#include <memory>
#include <set>
#include <sstream>
#include <vector>

namespace sergut {

class UrlSerializeToVector: public SerializerBase
{
public:
  UrlSerializeToVector(std::unique_ptr<UrlNameCombiner>&& nameCombiner = nullptr);
  ~UrlSerializeToVector();

  UrlSerializeToVector& operator&(const NamedMemberForSerialization<long long>& data) {
    writeSimpleType(_urlNameCombiner(misc::ConstStringRef(_structureName), misc::ConstStringRef(data.name)), data.data);
    return *this;
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<long>& data) {
    return operator&(toNamedMember(data.name, static_cast<long long>(data.data), data.mandatory));
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<int>& data) {
    return operator&(toNamedMember(data.name, static_cast<long long>(data.data), data.mandatory));
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<short>& data) {
    return operator&(toNamedMember(data.name, static_cast<long long>(data.data), data.mandatory));
  }

  UrlSerializeToVector& operator&(const NamedMemberForSerialization<unsigned long long>& data) {
    writeSimpleType(_urlNameCombiner(misc::ConstStringRef(_structureName), misc::ConstStringRef(data.name)), data.data);
    return *this;
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<unsigned long>& data) {
    return operator&(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<unsigned int>& data) {
    return operator&(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<unsigned short>& data) {
    return operator&(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<unsigned char>& data) {
    return operator&(toNamedMember(data.name, static_cast<unsigned long long>(data.data), data.mandatory));
  }

  UrlSerializeToVector& operator&(const NamedMemberForSerialization<double>& data) {
    writeSimpleType(_urlNameCombiner(misc::ConstStringRef(_structureName), misc::ConstStringRef(data.name)), data.data);
    return *this;
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<float>& data) {
    return operator&(toNamedMember(data.name, static_cast<double>(data.data), data.mandatory));
  }

  UrlSerializeToVector& operator&(const NamedMemberForSerialization<std::string>& data) {
    writeSimpleType(_urlNameCombiner(misc::ConstStringRef(_structureName), misc::ConstStringRef(data.name)),
                    data.data);
    return *this;
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<const char*>& data) {
    return operator&(toNamedMember(data.name, std::string(data.data), data.mandatory));
  }
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<char>& data) {
    return operator&(toNamedMember(data.name, std::string(1, data.data), data.mandatory));
  }


  // Containers as members
  template<typename DT>
  UrlSerializeToVector& serializeCollection(const NamedMemberForSerialization<DT>& data) {
    bool first=true;
    std::set<std::string> localSeenNames;
    for(auto&& value: data.data) {
      operator&(toNamedMember(data.name, value, true));
      if(first) {
        first = false;
        // after the first iteration, we clean the _seenNames
        // as we would definitely get duplicates
        //
        // this might lead to some duplicates not beeing detected
        // in some cases, but I neglect this for now as I am not
        // even sure I want to support collections in the first
        // place
        std::swap(_seenNames, localSeenNames);
      }
      _seenNames.clear();
      if(!localSeenNames.empty()) {
        // insert dummy element, such that the '&' will be written between the elements
        _seenNames.insert(std::string(1, '\0'));
      }
    }
    std::swap(localSeenNames, _seenNames);
    return *this;
  }

  template<typename ValueType>
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<std::vector<ValueType>>& data) {
    return serializeCollection(data);
  }

  template<typename ValueType>
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<std::list<ValueType>>& data) {
    return serializeCollection(data);
  }

  template<typename ValueType>
  UrlSerializeToVector& operator&(const NamedMemberForSerialization<std::set<ValueType>>& data) {
    return serializeCollection(data);
  }

  template<typename DT>
  auto operator&(const NamedMemberForSerialization<DT>& data)
  -> decltype(serialize(declval<UrlSerializeToVector&>(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)), *this)
  {
    UrlSerializeToVector ser(*this, misc::ConstStringRef(data.name));
    serialize(ser, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
    return *this;
  }

  // Members that can be converted to string
  template<typename DT>
  auto operator&(const NamedMemberForSerialization<DT>& data)
  -> decltype(serializeToString(data.data), *this)
  {
    return operator&(toNamedMember(data.name, serializeToString(data.data), data.mandatory));
  }

  // This is unused for URL encoding
  UrlSerializeToVector& operator&(const ChildrenFollow&) { return *this; }

  // This is unused for URL encoding
  UrlSerializeToVector& operator&(const PlainChildFollows&) { return *this; }

  template<typename DT>
  void serializeData(const std::string& name, const DT& data) {
    *this & toNamedMember(name.c_str(), data, true);
  }

  std::vector<std::pair<std::string, std::string>>&& takeParams() { return std::move(_out); }
  const std::vector<std::pair<std::string, std::string>>& getParams() const { return _out; }

private:
  UrlSerializeToVector(const UrlSerializeToVector& ref, const misc::ConstStringRef memberName);

  template<typename DT>
  void writeSimpleType(const std::string& name, const DT& data) {
    if(!_seenNames.insert(name).second) {
      throw SerializationException("Duplicate name");
    }
    std::ostringstream o;
    o << data;
    _out.push_back(std::make_pair(name, o.str()));
  }

private:
  std::vector<std::pair<std::string,std::string>> _ownOut;
  std::unique_ptr<UrlNameCombiner> _ownUrlNameCombiner;
  std::set<std::string> _ownSeenNames;
  std::vector<std::pair<std::string,std::string>>& _out;
  UrlNameCombiner& _urlNameCombiner;
  std::set<std::string>& _seenNames;
  std::string _structureName;
};

} // namespace sergut
