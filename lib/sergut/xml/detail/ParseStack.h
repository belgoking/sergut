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

#include "sergut/misc/StringRef.h"

#include <vector>

namespace sergut {
namespace xml {
namespace detail {

template<bool isUtf8>
class ParseStack;

template<>
class ParseStack<false>
{
public:
  ParseStack() { }

  void pushData(const sergut::misc::ConstStringRef& data) {
    const std::size_t tmpEnd = frameEnd.empty() ? 0 : frameEnd.back();
    if(buffer.size() < tmpEnd + data.size()) {
      buffer.resize(tmpEnd + data.size() + 50); // + 50 such that we don't have to do as many allocations
    }
    std::copy(data.begin(), data.end(), getTopFrameEnd());
    frameEnd.push_back(tmpEnd + data.size());
  }

  void popData() {
    frameEnd.pop_back();
  }

  std::size_t getTopFrameSize() const noexcept {
    if(buffer.empty()) { return 0; }
    return getTopFrameEnd() - getTopFrameStart();
  }

  const sergut::misc::ConstStringRef getTopData() const noexcept {
    return sergut::misc::ConstStringRef(getTopFrameStart(), getTopFrameEnd());
  }

  sergut::misc::StringRef getTopData() noexcept {
    sergut::misc::ConstStringRef tmp = const_cast<const ParseStack&>(*this).getTopData();
    return sergut::misc::StringRef(const_cast<char*>(tmp.begin()), const_cast<char*>(tmp.end()));
  }

  std::size_t frameCount() const noexcept {
    return frameEnd.size();
  }

  void addOffset(const std::ptrdiff_t offset) { (void)offset; }

private:
  const char* getTopFrameStart() const noexcept {
    if(frameEnd.size() < 2) {
      return &*buffer.begin();
    }
    return &*(buffer.begin() + *(frameEnd.rbegin()+1) );
  }
  char* getTopFrameStart() noexcept {
    return const_cast<char*>(const_cast<const ParseStack&>(*this).getTopFrameStart());
  }

  const char* getTopFrameEnd() const noexcept {
    if(frameEnd.empty()) {
      return &*buffer.begin();
    }
    return &*(buffer.begin() + *frameEnd.rbegin() );
  }
  char* getTopFrameEnd() noexcept {
    return const_cast<char*>(const_cast<const ParseStack&>(*this).getTopFrameEnd());
  }

private:
  std::vector<char> buffer;
  std::vector<std::size_t> frameEnd;
};

template<>
class ParseStack<true>
{
public:
  ParseStack() { }

  void pushData(const sergut::misc::ConstStringRef& data) {
    frames.push_back(data);
  }

  void popData() {
    frames.pop_back();
  }

  const sergut::misc::ConstStringRef getTopData() const noexcept {
    if(frames.empty()) {
      return sergut::misc::ConstStringRef();
    }
    return frames.back();
  }

  std::size_t frameCount() const noexcept {
    return frames.size();
  }

  void addOffset(const std::ptrdiff_t offset) {
    for(sergut::misc::ConstStringRef& frame: frames) {
      frame.addOffset(offset);
    }
  }

private:
  std::vector<sergut::misc::ConstStringRef> frames;
};

}
}
}
