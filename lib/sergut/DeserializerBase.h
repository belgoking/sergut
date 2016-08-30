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

namespace sergut {

template<typename DT>
struct NamedMemberForDeserialization {
  typedef DT value_type;
  NamedMemberForDeserialization(const char* pName, DT& pData, bool pMandatory)
    : name(pName)
    , data(pData)
    , mandatory(pMandatory)
  { }
  const char* name;
  DT& data;
  const bool mandatory;
};

// This function is used for framed datastructures.
// E.g. An XML-List that is surrounded by an additional Tag.
template<typename DT, typename Archive, typename InnerType>
void serialize(Archive& ar, DT& data, const NamedMemberForDeserialization<InnerType>*)
{
  ar & sergut::children & data;
}


template<typename DT>
struct NamedMemberForDeserialization<NamedMemberForDeserialization<DT>> {
  typedef DT value_type;
  NamedMemberForDeserialization(const char* pName, const NamedMemberForDeserialization<DT>& pData, bool pMandatory)
    : name(pName)
    , data(pData)
    , mandatory(pMandatory)
  { }
  const char* name;
  NamedMemberForDeserialization<DT> data;
  const bool mandatory;
};


class DeserializerBase {
public:
  template<typename DT>
  static NamedMemberForDeserialization<DT> toNamedMember(const char* name, DT& data, const bool mandatory) {
    return NamedMemberForDeserialization<DT>(name, data, mandatory);
  }
  template<typename DT>
  static NamedMemberForDeserialization<NamedMemberForDeserialization<DT>> toNamedMember(const char* name, const NamedMemberForDeserialization<DT>& data, bool mandatory) {
    return NamedMemberForDeserialization<NamedMemberForDeserialization<DT>>(name, data, mandatory);
  }
};

struct DummyDeserializer : public DeserializerBase {
  template<typename T>
  const DummyDeserializer& operator&(T) const { return *this; }
  static DummyDeserializer& dummyInstance(); ///< This is not implemented
};

}
