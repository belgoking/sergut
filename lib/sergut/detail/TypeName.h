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

#include "sergut/detail/NameSpace.h"

#include <iosfwd>
#include <string>

namespace sergut {
namespace detail {

struct TypeName {
  enum class CollectionType {
    None,
    Array,
    Set
  };

  TypeName() = default;
  TypeName(const NameSpace& nameSpace, const std::string& qualifiedName, const CollectionType collectionType);
  TypeName(const std::string& qualifiedName, const CollectionType collectionType);

  TypeName toCollectionType() const;
  TypeName toElementType() const;

  CollectionType collectionType = CollectionType::None;
  NameSpace nameSpace;
  std::string typeName;
};

std::ostream& operator<<(std::ostream& ostr, const TypeName::CollectionType& collectionType);
std::ostream& operator<<(std::ostream& ostr, const TypeName& typeName);

} // namespace detail
} // namespace sergut
