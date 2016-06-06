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

#include <cinttypes>

namespace xml {

enum class ParseTokenType: uint32_t {
  InitialState       = static_cast<uint32_t>(-1),
  OpenDocument       = static_cast<uint32_t>(-2),
  OpenTag            = static_cast<uint32_t>(-3),
  Attribute          = static_cast<uint32_t>(-4),
  Text               = static_cast<uint32_t>(-5),
  CloseTag           = static_cast<uint32_t>(-6),
  CloseDocument      = static_cast<uint32_t>(-7),
  IncompleteDocument = static_cast<uint32_t>(-8),
  Error              = static_cast<uint32_t>(-9)
//  ProcessingInstruction,
//  Comment,
};

inline
bool isOk(const ParseTokenType tokenType)
{
  return tokenType != ParseTokenType::IncompleteDocument && tokenType != ParseTokenType::Error;
}

}
