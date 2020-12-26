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
#include "sergut/XmlValueType.h"
#include "sergut/detail/Nesting.h"

#include <type_traits>

namespace sergut {

template<typename DT>
struct NamedMemberForDeserialization: public detail::DataHolder<DT> {
  typedef DT value_type;
  NamedMemberForDeserialization(const char* pName, DT& pData, const bool pMandatory)
    : detail::DataHolder<DT>{pData}
    , name(pName)
    , mandatory(pMandatory)
  { }
  const char* name;
  const bool mandatory;
};

namespace detail {
template<typename DT>
struct DataHolder<NamedMemberForDeserialization<DT>> {
  NamedMemberForDeserialization<DT> data;
};
} // namespace detail

class DeserializerBase {
public:
  template<typename DT>
  static
  NamedMemberForDeserialization<
          typename std::remove_reference<
          typename std::remove_cv<DT>::type>::type>
  toNamedMember(const char* name, DT&& data, const bool mandatory)
  {
    return NamedMemberForDeserialization<
        typename std::remove_reference<
        typename std::remove_cv<DT>::type>::type>(name, data, mandatory);
  }

  template<typename DT>
  static
  detail::Nesting<
          typename std::remove_reference<
          typename std::remove_cv<DT>::type>::type>
  toNestedMember(const char* name, DT&& data, const bool mandatory,
                 const XmlValueType pXmlValueType = XmlValueType::Child)
  {
    return detail::Nesting<
        typename std::remove_reference<
        typename std::remove_cv<DT>::type>::type>(name, data, mandatory, pXmlValueType);
  }
};

}
