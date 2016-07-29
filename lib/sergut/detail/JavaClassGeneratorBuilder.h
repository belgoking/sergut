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

#include "sergut/detail/JavaClassGeneratorBase.h"
#include "sergut/detail/TypeName.h"

namespace sergut {
namespace detail {

template<typename DerivedClass>
class JavaClassGeneratorBuilder: public detail::JavaClassGeneratorBase
{
public:
  template<typename DataType, typename ...DerivedClassConstructorArgs>
  static DerivedClass generate(DerivedClassConstructorArgs&& ...args) {
    DerivedClass ret(std::forward(args)...);
    DataType dummy;
    ret.typeName = detail::TypeName(std::string(getTypeName(&dummy)), detail::TypeName::CollectionType::None); // ToDo Implement
    serialize(ret, dummy, static_cast<DataType*>(nullptr));
    return ret;
  }
};

} // namespace detail
} // namespace sergut
