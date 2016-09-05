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

#include "sergut/XmlValueType.h"

namespace sergut {
namespace detail {
// Helper type to hold reference to lValues and copies to rValue types
// (specifically NamedMemberForDeserialization and Nested)
template<typename DT>
struct DataHolder {
  DT& data;
};

template<typename DT>
struct Nesting : DataHolder<DT> {
  typedef DT value_type;
  Nesting(const char* pName, DT& pData, const bool pMandatory,
          const XmlValueType pXmlValueType)
    : DataHolder<DT>{pData}
    , name(pName)
    , mandatory(pMandatory)
    , xmlValueType(pXmlValueType)
  { }
  const char* name;
  const bool mandatory;
  const XmlValueType xmlValueType;
};

template<typename DT>
struct DataHolder<Nesting<DT>> {
  Nesting<DT> data;
};

// This function is used for nested datastructures.
// E.g. An XML-List that is surrounded by an additional Tag.
template<typename DT, typename Archive, typename InnerType>
void serialize(Archive& ar, DT& data, const Nesting<InnerType>*)
{
  switch(data.xmlValueType) {
  case XmlValueType::Attribute:
    break;
  case XmlValueType::Child:
    ar & sergut::children;
    break;
  case XmlValueType::SingleChild:
    ar & sergut::plainChild;
    break;
  }
  ar & Archive::toNamedMember(data.name, data.data, data.mandatory);
}

} // namespace detail
} // namespace sergut
