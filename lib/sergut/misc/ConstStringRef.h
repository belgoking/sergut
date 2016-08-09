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

#include <string>
#include <iosfwd>

namespace sergut {
namespace misc {

class ConstStringRef
{
public:
  typedef const char* const_iterator;
  ConstStringRef() noexcept : beginPtr(), endPtr() { }
  ConstStringRef(const char* pBegin, const char* pEnd) noexcept
    : beginPtr(pBegin), endPtr(pEnd)
  { }
  template<std::size_t N>
  explicit ConstStringRef(const char(&str)[N]) noexcept : beginPtr(str), endPtr(str+N-1) { }
  explicit ConstStringRef(const std::string& str) noexcept
    : beginPtr(&*str.begin()), endPtr(&*str.end())
  { }

  std::size_t size() const noexcept { return endPtr - beginPtr; }
  bool empty() const noexcept { return size() == 0; }

  const_iterator begin() const noexcept { return beginPtr; }
  const_iterator end()   const noexcept { return endPtr;   }

  const char& operator[](const std::size_t pos) const { return  *(beginPtr + pos); }

  std::string toString() const { return std::string(beginPtr, endPtr); }

  bool consumeFront(const std::size_t charsToConsume) noexcept {
    if(beginPtr + charsToConsume > endPtr) {
      beginPtr = endPtr;
      return false;
    }
    beginPtr += charsToConsume;
    return true;
  }

private:
  friend bool operator==(const ConstStringRef& lhs, const ConstStringRef& rhs) noexcept;

protected:
  const char* beginPtr;
  const char* endPtr;
};

inline
bool operator==(const ConstStringRef& lhs, const ConstStringRef& rhs) noexcept {
  if(lhs.size() != rhs.size()) {
    return false;
  }
  ConstStringRef::const_iterator lhsPos = lhs.begin();
  ConstStringRef::const_iterator rhsPos = rhs.begin();
  for(; lhsPos != lhs.end(); ++lhsPos, ++rhsPos) {
    if(*lhsPos != *rhsPos) {
      return false;
    }
  }
  return true;
}

inline
bool operator==(const ConstStringRef& lhs, const std::string& rhs) noexcept {
  return lhs == ConstStringRef(rhs);
}

inline
bool operator==(const std::string& lhs, const ConstStringRef& rhs) noexcept {
  return ConstStringRef(lhs) == rhs;
}

inline
bool operator!=(const ConstStringRef& lhs, const ConstStringRef& rhs) noexcept {
  return !operator==(lhs, rhs);
}

inline
bool operator!=(const ConstStringRef& lhs, const std::string& rhs) noexcept {
  return !operator==(lhs, rhs);
}

inline
bool operator!=(const std::string& lhs, const ConstStringRef& rhs) noexcept {
  return !operator==(lhs, rhs);
}

inline std::ostream& operator<<(std::ostream& out, const ConstStringRef& str) {
  out << std::string(str.begin(), str.end());
  return out;
}
}
}
