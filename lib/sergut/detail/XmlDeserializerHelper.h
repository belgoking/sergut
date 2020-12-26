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
#include "sergut/detail/TypeName.h"

#include <list>
#include <set>
#include <vector>

namespace sergut {
namespace detail {

struct XmlDeserializerHelper {
  template<typename DT>
  static constexpr bool canDeserializeIntoAttribute() { return doCanDeserializeIntoAttribute(static_cast<DT*>(nullptr), 0L); }

private:
  template<typename DT>
  static constexpr DT& getHolder();

  template<typename DT>
  static constexpr auto doCanDeserializeIntoAttribute(const DT*, const char) -> bool { return true; }

  template<typename DT>
  static constexpr auto doCanDeserializeIntoAttribute(const DT*, const long)
  -> decltype(serialize(declval<XmlDeserializerHelper&>(), getHolder<DT>(), static_cast<typename std::decay<DT>::type*>(nullptr)),bool())
  { return false; }

  template<typename DT>
  static constexpr auto doCanDeserializeIntoAttribute(const std::list<DT>*, const long) -> bool
  { return false; }

  template<typename DT>
  static constexpr auto doCanDeserializeIntoAttribute(const std::vector<DT>*, const long) -> bool
  { return false; }

  template<typename DT>
  static constexpr auto doCanDeserializeIntoAttribute(const std::set<DT>*, const long) -> bool
  { return false; }
};

} // namespace detail
} // namespace sergut
