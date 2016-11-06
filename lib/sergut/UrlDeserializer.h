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
#include "sergut/UrlNameCombiner.h"
#include "sergut/ParsingException.h"
#include "sergut/Util.h"
#include "sergut/detail/XmlDeserializerHelper.h"
#include "sergut/detail/DummySerializer.h"
#include "sergut/misc/ReadHelper.h"

#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace sergut {

/**
 * \brief Deserializer from URL-Parameters that is fead from a multimap
 */
class UrlDeserializer: public DeserializerBase
{
public:
  /**
   * @brief UrlDeserializer Construct from initial multimap
   * @param params The multimap with the URL-Parameters
   * @param urlNameCombiner The \c UrlNameCombiner that is used to join the url-names
   */
  UrlDeserializer(const std::vector<std::pair<std::string,std::string>>& params,
                  std::unique_ptr<UrlNameCombiner>&& urlNameCombiner = nullptr);
  /**
   * @brief UrlDeserializer Construct from initial multimap
   * @param params The multimap with the URL-Parameters
   * @param urlNameCombiner The \c UrlNameCombiner that is used to join the url-names
   */
  UrlDeserializer(std::vector<std::pair<std::string,std::string>>&& params,
                  std::unique_ptr<UrlNameCombiner>&& urlNameCombiner = nullptr);

  /**
   * \brief Deserialize data into type \c DT
   * \param name The name of the outer tag.
   * \tparam DT The type into which the URL should be deserialized.
   */
  template<typename DT>
  DT deserializeData(const char* name) {
    DT data;
    *this & toNamedMember(name, data, true);
    return data;
  }

public: // The archive operator& that is used by the \c serialize() functions.
  template<typename T>
  typename std::enable_if<std::is_arithmetic<T>::value, UrlDeserializer&>::type
  operator&(const NamedMemberForDeserialization<T>& data) {
    extractSimpleType(data);
    return *this;
  }

  // String types
  UrlDeserializer& operator&(const NamedMemberForDeserialization<std::string>& data) {
    extractSimpleType(data);
    return *this;
  }


  UrlDeserializer& operator&(const NamedMemberForDeserialization<const char*>& data) = delete;

  // Containers as members
  // I prevent collections of structured classes for now, as they are non-trivial to implement
  // and some some implementations are even quite susceptible to DoS-Attacks.
  template<typename CDT>
  UrlDeserializer& operator&(const NamedMemberForDeserialization<std::vector<CDT>>& data) {
    static_assert(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<CDT>(),
                  "Vectors of structured types are not supported");
    const std::string fullName = _urlNameCombiner(misc::ConstStringRef(_structureName), misc::ConstStringRef(data.name));
    while(findParam(fullName) != _params.end()) {
      CDT tmp;
      *this & toNamedMember(data.name, tmp, true);
      data.data.push_back(tmp);
    }
    return *this;
  }

  template<typename CDT>
  UrlDeserializer& operator&(const NamedMemberForDeserialization<std::list<CDT>>& data) {
    static_assert(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<CDT>(),
                  "Lists of structured types are not supported");
    const std::string fullName = _urlNameCombiner(misc::ConstStringRef(_structureName), misc::ConstStringRef(data.name));
    while(findParam(fullName) != _params.end()) {
      CDT tmp;
      *this & toNamedMember(data.name, tmp, true);
      data.data.push_back(tmp);
    }
    return *this;
  }

  template<typename CDT>
  UrlDeserializer& operator&(const NamedMemberForDeserialization<std::set<CDT>>& data) {
    static_assert(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<CDT>(),
                  "Sets of structured types are not supported");
    const std::string fullName = _urlNameCombiner(misc::ConstStringRef(_structureName), misc::ConstStringRef(data.name));
    while(findParam(fullName) != _params.end()) {
      CDT tmp;
      *this & toNamedMember(data.name, tmp, true);
      data.data.insert(tmp);
    }
    return *this;
  }

  // Members that can be converted to string
  template<typename DT>
  auto operator&(const NamedMemberForDeserialization<DT>& data)
  -> decltype(deserializeFromString(data.data, std::string()),*this)
  {
    std::string tmpString;
    if(extractSimpleType(toNamedMember(data.name, tmpString, data.mandatory))) {
      deserializeFromString(data.data, tmpString);
    }
    return *this;
  }

  template<typename DT>
  auto operator&(const NamedMemberForDeserialization<DT>& data)
  -> decltype(serialize(DummyDeserializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)),*this)
  {
    UrlDeserializer ser(*this, misc::ConstStringRef(data.name));
    serialize(ser, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
    return *this;
  }

  // This does nothing for URL deserialization
  UrlDeserializer& operator&(const ChildrenFollow&) { return *this; }

  // This does nothing for URL deserialization
  UrlDeserializer& operator&(const PlainChildFollows&) { return *this; }

private:
  UrlDeserializer(const UrlDeserializer& ref, const misc::ConstStringRef memberName);

  std::vector<std::pair<std::string,std::string>>::iterator
  findParam(const std::string& toFind)
  {
    for(auto it = _params.begin(); it != _params.end(); ++it) {
      if(it->first == toFind) {
        return it;
      }
    }
    return _params.end();
  }

  template<typename DT>
  bool extractSimpleType(const NamedMemberForDeserialization<DT>& data) {
    const std::string fullName = _urlNameCombiner(misc::ConstStringRef(_structureName), misc::ConstStringRef(data.name));
    const auto it = findParam(fullName);
    if(it == _params.end()) {
      if(data.mandatory) {
        throw ParsingException("Missing mandatory URL parameter");
      }
      return false;
    }
    misc::ReadHelper::readInto(misc::ConstStringRef(it->second), data.data);
    _params.erase(it);
    return true;
  }

private:
  std::unique_ptr<UrlNameCombiner> _ownUrlNameCombiner;
  std::vector<std::pair<std::string,std::string>> _ownParams;
  UrlNameCombiner& _urlNameCombiner;
  std::vector<std::pair<std::string,std::string>>& _params;
  std::string _structureName;
};

} // namespace sergut
