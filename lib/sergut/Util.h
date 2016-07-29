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

#define SERGUT_MMEMBER(cls, mem) \
  Archive::toNamedMember(#mem, cls.mem, true)

#define SERGUT_OMEMBER(cls, mem) \
  Archive::toNamedMember(#mem, cls.mem, false)

#define SERGUT_NESTED_MMEMBER(cls, mem, innerName) \
  Archive::toNamedMember(#mem, Archive::toNamedMember(#innerName, cls.mem, true), true)

#define SERGUT_NESTED_OMEMBER(cls, mem, innerName) \
  Archive::toNamedMember(#mem, Archive::toNamedMember(#innerName, cls.mem, false), false)

#define SERGUT_FUNCTION(DataType, dataName, archiveName) \
  inline const char* getTypeName(const DataType*) { return #DataType; } \
  template<typename DT, typename Archive> \
  void serialize(Archive& archiveName, DT& dataName, const DataType*)

#define SERGUT_FUNCTION_FRIEND_DECL(DataType, dataName, archiveName) \
  template<typename DT, typename Archive> \
  friend \
  void serialize(Archive& archiveName, DT& dataName, const DataType*)

#define SERGUT_SERIALIZE_TO_STRING(DT, variableName) \
  inline const char* getTypeName(const DT*) { return #DT; } \
  inline std::string serializeToString(const DT& variableName)

#define SERGUT_SERIALIZE_TO_STRING_FRIEND_DECL(DT, variableName) \
  friend std::string serializeToString(const DT& variableName)

#define SERGUT_DESERIALIZE_FROM_STRING(DT, variableName, stringVariableName) \
  inline void deserializeFromString(DT& variableName, const std::string& stringVariableName)

#define SERGUT_DESERIALIZE_FROM_STRING_FRIEND_DECL(DT, variableName, stringVariableName) \
  friend void deserializeFromString(DT& variableName, const std::string& stringVariableName)

namespace sergut {

struct ChildrenFollow {};
const ChildrenFollow children;

struct PlainChildFollows {};
const PlainChildFollows plainChild;

}
