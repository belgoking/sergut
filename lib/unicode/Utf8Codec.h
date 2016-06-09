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

#include "misc/StringRef.h"
#include "unicode/ParseResult.h"
#include "unicode/Utf32Char.h"

#include <string>

namespace unicode {

struct Utf8Codec {
  static ParseResult parseNext(Utf32Char& chr, const char* data, const char* dataEnd) noexcept;
  static bool isAscii(const char c) noexcept;
  static unicode::ParseResult encodeChar(const unicode::Utf32Char chr, char* bufStart = nullptr, const char* bufEnd = nullptr) noexcept;
  static ParseResult appendChar(std::string& out, const Utf32Char chr);
  static bool hasBom(const char* data, const char* dataEnd) noexcept;
  static bool isSupportedEncoding(const misc::ConstStringRef& encodingString) noexcept;
};

}
