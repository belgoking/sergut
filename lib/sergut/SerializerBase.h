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

#include "sergut/Util.h"
#include "sergut/detail/Nesting.h"

#include <utility>

namespace sergut {

template<typename DT>
struct NamedMemberForSerialization {
  NamedMemberForSerialization(const char* pName, const DT& pData, bool pMandatory)
    : name(pName)
    , data(pData)
    , mandatory(pMandatory)
  { }
  const char* name;
  const DT& data;
  const bool mandatory;
};

template<typename DiscriminatorType, typename HandlerType>
struct ObjectByKeyValueForSerialization {
  ObjectByKeyValueForSerialization(const char* k, const char* v, DiscriminatorType d, HandlerType h)
    : key(k)
    , value(v)
    , discriminator(std::move(d))
    , handler(std::move(h))
  { }

  const char* key;
  const char* value;
  DiscriminatorType discriminator;
  HandlerType handler;
};

template<typename DiscriminatorType, typename HandlerType>
struct ObjectByKeyForSerialization {
  ObjectByKeyForSerialization(const char* k, DiscriminatorType d, HandlerType h)
    : key(k)
    , discriminator(std::move(d))
    , handler(std::move(h))
  { }

  const char* key;
  DiscriminatorType discriminator;
  HandlerType handler;
};

class SerializerBase {
public:
  template<typename DT>
  static NamedMemberForSerialization<DT> toNamedMember(const char* name, const DT& data, const bool mandatory)
  {
    return NamedMemberForSerialization<DT>(name, data, mandatory);
  }

  template<typename DT>
  static detail::Nesting<const DT> toNestedMember(const char* name, const DT& data, const bool mandatory,
                                                        const XmlValueType pXmlValueType = XmlValueType::Child)
  {
    return detail::Nesting<const DT>(name, data, mandatory, pXmlValueType);
  }

  template<typename DiscriminatorType, typename HandlerType>
  static
  ObjectByKeyValueForSerialization<DiscriminatorType, HandlerType>
  objectByKeyValue(const char* key, const char* value,
                   DiscriminatorType discriminator, HandlerType handler)
  {
    return ObjectByKeyValueForSerialization<DiscriminatorType, HandlerType>(
          key, value, std::move(discriminator), std::move(handler));
  }

  template<typename DiscriminatorType, typename HandlerType>
  static
  ObjectByKeyForSerialization<DiscriminatorType, HandlerType>
  objectByKey(const char* key, DiscriminatorType discriminator, HandlerType handler)
  {
    return ObjectByKeyForSerialization<DiscriminatorType, HandlerType>(
          key, std::move(discriminator), std::move(handler));
  }
};

}
