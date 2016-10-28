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

namespace sergut {
namespace misc {

struct DataType {
  enum Type {
    Bool,
    Char,
    UnsignedChar,
    SignedChar,
    Short,
    UnsignedShort,
//    SignedShort,
    Int,
    UnsignedInt,
//    SignedInt,
    Long,
    UnsignedLong,
//    SignedLong,
    LongLong,
    UnsignedLongLong,
//    SignedLongLong,
    Float,
    Double,
    LongDouble,
    String,
    Other
  };

  template<typename DT> static Type getDataType() { return Other; }

  static const char* getName(Type t) {
    switch(t) {
    case Bool:             return "Bool";
    case Char:             return "Char";
    case UnsignedChar:     return "UnsignedChar";
    case SignedChar:       return "SignedChar";
    case Short:            return "Short";
    case UnsignedShort:    return "UnsignedShort";
//    case SignedShort:      return "SignedShort";
    case Int:              return "Int";
    case UnsignedInt:      return "UnsignedInt";
//    case SignedInt:        return "SignedInt";
    case Long:             return "Long";
    case UnsignedLong:     return "UnsignedLong";
//    case SignedLong:       return "SignedLong";
    case LongLong:         return "LongLong";
    case UnsignedLongLong: return "UnsignedLongLong";
//    case SignedLongLong:   return "SignedLongLong";
    case Float:            return "Float";
    case Double:           return "Double";
    case LongDouble:       return "LongDouble";
    case String:           return "String";
    case Other:
    default:
      return "Other";
    }
  }
};

template<> inline DataType::Type DataType::getDataType<bool>() { return Bool; }
template<> inline DataType::Type DataType::getDataType<char>() { return Char; }
template<> inline DataType::Type DataType::getDataType<unsigned char>() { return UnsignedChar; }
template<> inline DataType::Type DataType::getDataType<signed char>() { return SignedChar; }
template<> inline DataType::Type DataType::getDataType<short>() { return Short; }
template<> inline DataType::Type DataType::getDataType<unsigned short>() { return UnsignedShort; }
//template<> inline DataType::Type DataType::getDataType<signed short>() { return SignedShort; }
template<> inline DataType::Type DataType::getDataType<int>() { return Int; }
template<> inline DataType::Type DataType::getDataType<unsigned int>() { return UnsignedInt; }
//template<> inline DataType::Type DataType::getDataType<signed int>() { return SignedInt; }
template<> inline DataType::Type DataType::getDataType<long>() { return Long; }
template<> inline DataType::Type DataType::getDataType<unsigned long>() { return UnsignedLong; }
//template<> inline DataType::Type DataType::getDataType<signed long>() { return SignedLong; }
template<> inline DataType::Type DataType::getDataType<long long>() { return LongLong; }
template<> inline DataType::Type DataType::getDataType<unsigned long long>() { return UnsignedLongLong; }
//template<> inline DataType::Type DataType::getDataType<signed long long>() { return SignedLongLong; }
template<> inline DataType::Type DataType::getDataType<float>() { return Float; }
template<> inline DataType::Type DataType::getDataType<double>() { return Double; }
template<> inline DataType::Type DataType::getDataType<long double>() { return LongDouble; }
template<> inline DataType::Type DataType::getDataType<std::string>() { return String; }
template<> inline DataType::Type DataType::getDataType<char*>() { return String; }

}
}
