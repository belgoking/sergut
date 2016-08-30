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

// This function is used for framed datastructures.
// E.g. An XML-List that is surrounded by an additional Tag.
template<typename DT, typename Archive, typename InnerType>
void serialize(Archive& ar, DT& data, const NamedMemberForSerialization<InnerType>*)
{
  ar & sergut::children & data;
}



class SerializerBase {
public:
  template<typename DT>
  static NamedMemberForSerialization<DT> toNamedMember(const char* name, const DT& data, const bool mandatory) {
    return NamedMemberForSerialization<DT>(name, data, mandatory);
  }
};

struct DummySerializer : public SerializerBase {
  template<typename T>
  const DummySerializer& operator&(T) const { return *this; }
  static DummySerializer& dummyInstance(); ///< This is not implemented
};

}
