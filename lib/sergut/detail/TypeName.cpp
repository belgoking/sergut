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

#include "sergut/detail/TypeName.h"
#include "sergut/SerializationException.h"

#include <ostream>

namespace sergut {
namespace detail {

static bool skipScope(std::string::const_iterator& it, const std::string& name) {
  ++it;
  if(it == name.cend() || *it != ':') {
    return false;
  }
  ++it;
  return true;
}

TypeName::TypeName(const NameSpace& ns, const std::string& tn, const TypeName::CollectionType collType)
  : collectionType(collType)
  , nameSpace(ns)
  , typeName(tn)
{ }

TypeName::TypeName(const std::string& qualifiedName, const CollectionType collType)
  : collectionType(collType)
{
  std::string::const_iterator currentIt = qualifiedName.cbegin();
  // skip leading '::'
  if(currentIt == qualifiedName.cend() || (*currentIt == ':' && !skipScope(currentIt, qualifiedName))) {
    throw SerializationException("'" + qualifiedName + "' is not a correct typename");
  }
  std::string::const_iterator lastIt = currentIt;
  while(currentIt != qualifiedName.cend()) {
    if(*currentIt == ':') {
      if(currentIt == lastIt) {
        throw SerializationException("'" + qualifiedName + "' is not a correct typename");
      }
      nameSpace.push_back(std::string(lastIt, currentIt));
      if(!skipScope(currentIt, qualifiedName)) {
        throw SerializationException("'" + qualifiedName + "' is not a correct typename");
      }
      lastIt = currentIt;
      continue;
    }
    ++currentIt;
  }
  if(currentIt == lastIt) {
    throw SerializationException("'" + qualifiedName + "' is not a correct typename");
  }
  typeName = std::string(lastIt, currentIt);
}

TypeName TypeName::toCollectionType() const
{
  switch(collectionType) {
  case TypeName::CollectionType::Array:
    return TypeName{{"java", "util"}, "ArrayList", CollectionType::None};    break;
  case TypeName::CollectionType::Set:
    return TypeName{{"java", "util"}, "HashSet", CollectionType::None};    break;
  case TypeName::CollectionType::None:
    break;
  };
  return TypeName{};
}

TypeName TypeName::toElementType() const
{
  TypeName ret = *this;
  ret.collectionType = CollectionType::None;
  return ret;
}

std::ostream& operator<<(std::ostream& ostr, const TypeName::CollectionType& collectionType)
{
  switch(collectionType) {
  case TypeName::CollectionType::Array:
    ostr << "java.util.ArrayList";
    break;
  case TypeName::CollectionType::Set:
    ostr << "java.util.HashSet";
    break;
  case TypeName::CollectionType::None:
    break;
  };
  return ostr;
}

std::ostream& operator<<(std::ostream& ostr, const TypeName& typeName) {
  bool isCollection = typeName.collectionType != TypeName::CollectionType::None;
  if(isCollection) {
    ostr << typeName.collectionType << "<";
  }
  for(const std::string& nse: typeName.nameSpace) {
    ostr << nse << '.';
  }
  ostr << typeName.typeName;
  if(isCollection) {
    ostr << '>';
  }
  return ostr;
}

} // namespace detail
} // namespace sergut
