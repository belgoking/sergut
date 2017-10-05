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

#include <cinttypes>
#include <iomanip>
#include <sstream>

struct SomeTestData {
  SomeTestData() = default;
  SomeTestData(const uint32_t m, const uint32_t o) : mandatoryMember(m), optionalMember(o) { }
  bool operator==(const SomeTestData& rhs) const
  {
    return mandatoryMember == rhs.mandatoryMember &&
        optionalMember == rhs.optionalMember;
  }
  uint32_t mandatoryMember = 0;
  uint32_t optionalMember = 0;
};
SERGUT_FUNCTION(SomeTestData, data, ar) {
  ar & sergut::children
      & SERGUT_MMEMBER(data, mandatoryMember)
      & SERGUT_OMEMBER(data, optionalMember);
}

struct SingleChildTestData {
  SingleChildTestData() = default;
  SingleChildTestData(const uint32_t m, const uint32_t o) : attributeMember(m), singleChildMember(o) { }
  bool operator==(const SingleChildTestData& rhs) const
  {
    return attributeMember == rhs.attributeMember &&
        singleChildMember == rhs.singleChildMember;
  }
  uint32_t attributeMember = 0;
  uint32_t singleChildMember = 0;
};
SERGUT_FUNCTION(SingleChildTestData, data, ar) {
  ar
      & SERGUT_MMEMBER(data, attributeMember)
      & sergut::plainChild
      & SERGUT_OMEMBER(data, singleChildMember);
}

class Time {
public:
  Time(const std::uint8_t hour = 0, const std::uint8_t minute = 0, const std::uint8_t seconds = 0) : val(hour*10000+minute*100+seconds) { }

  uint32_t getInternalValue() const { return val; }
  bool operator==(const Time& rhs) const { return val == rhs.val; }
  bool operator<(const Time& rhs) const { return val < rhs.val; }
private:
  SERGUT_SERIALIZE_TO_STRING_FRIEND_DECL(Time, time);
  SERGUT_DESERIALIZE_FROM_STRING_FRIEND_DECL(Time, time, str);
  std::uint32_t val;
};

SERGUT_SERIALIZE_TO_STRING(Time, time)
{
  std::ostringstream tmp;
  tmp << std::setfill('0') << time.val / 10000 << ":" << std::setw(2) << time.val / 100 % 100 << ":" << std::setw(2) << time.val % 100;
  return tmp.str();
}

SERGUT_DESERIALIZE_FROM_STRING(Time, time, str)
{
  std::istringstream tmp(str);
  std::uint32_t hours;
  tmp >> hours;
  char dummyChar;
  tmp >> dummyChar;
  std::uint32_t minutes;
  tmp >> minutes;
  tmp >> dummyChar;
  std::uint32_t seconds;
  tmp >> seconds;
  time.val = hours * 10000 + minutes * 100 + seconds;
}

class SomeMoreComplexTestData {
public:
  SomeMoreComplexTestData() = default;
  SomeMoreComplexTestData(const Time& aTime, const char aSomeLetter,
                          const std::uint16_t aSomeUnsignedShortInt,
                          const Time& aMoreTime)
    : time(aTime)
    , someLetter(aSomeLetter)
    , someUnsignedShortInt(aSomeUnsignedShortInt)
    , moreTime(aMoreTime)
  { }

  Time getTime() const;
  void setTime(const Time& value);

  char getSomeLetter() const;
  void setSomeLetter(char value);

  std::uint16_t getSomeUnsignedShortInt() const;
  void setSomeUnsignedShortInt(const std::uint16_t& value);

  Time getMoreTime() const;
  void setMoreTime(const Time& value);
  bool operator==(const SomeMoreComplexTestData& rhs) const
  {
    return time == rhs.time && someLetter == rhs.someLetter &&
        someUnsignedShortInt == rhs.someUnsignedShortInt && moreTime == rhs.moreTime;
  }
private:
  SERGUT_FUNCTION_FRIEND_DECL(SomeMoreComplexTestData, data, ar);
  Time time;
  char someLetter;
  std::uint16_t someUnsignedShortInt;
  Time moreTime;
};

SERGUT_FUNCTION(SomeMoreComplexTestData, data, ar) {
  ar
      & SERGUT_MMEMBER(data, time)
      & SERGUT_MMEMBER(data, someLetter)
      & SERGUT_MMEMBER(data, someUnsignedShortInt)
      & SERGUT_MMEMBER(data, moreTime);
}

inline
Time SomeMoreComplexTestData::getTime() const
{
  return time;
}

inline
void SomeMoreComplexTestData::setTime(const Time& value)
{
  time = value;
}

inline
char SomeMoreComplexTestData::getSomeLetter() const
{
  return someLetter;
}

inline
void SomeMoreComplexTestData::setSomeLetter(char value)
{
  someLetter = value;
}

inline
std::uint16_t SomeMoreComplexTestData::getSomeUnsignedShortInt() const
{
  return someUnsignedShortInt;
}

inline
void SomeMoreComplexTestData::setSomeUnsignedShortInt(const std::uint16_t& value)
{
  someUnsignedShortInt = value;
}

inline
Time SomeMoreComplexTestData::getMoreTime() const
{
  return moreTime;
}

inline
void SomeMoreComplexTestData::setMoreTime(const Time& value)
{
  moreTime = value;
}

namespace {
class TestChild2
{
public:
  TestChild2(int pGrandChildValue = 0) : grandChildValue(pGrandChildValue) { }
  bool operator==(const TestChild2& rhs) const {
    return grandChildValue == rhs.grandChildValue;
  }
private:
  SERGUT_FUNCTION_FRIEND_DECL(TestChild2, data, ar);
  int grandChildValue;
};

SERGUT_FUNCTION(TestChild2, data, ar)
{
  ar & SERGUT_MMEMBER(data, grandChildValue);
}

struct TestChild
{
public:
  bool operator==(const TestChild& rhs) const {
    return
        intMember1    == rhs.intMember1    &&
        intMember2    == rhs.intMember2    &&
        timeMember3   == rhs.timeMember3   &&
        intMember4    == rhs.intMember4    &&
        doubleMember5 == rhs.doubleMember5 &&
        floatMember6  == rhs.floatMember6  &&
        intMember7    == rhs.intMember7;

  }
public:
  long long intMember1;
  long intMember2;
  Time timeMember3;
  int intMember4;
  double doubleMember5;
  float floatMember6;
  short intMember7;
};

SERGUT_FUNCTION(TestChild, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, intMember1)
      & SERGUT_MMEMBER(data, intMember2)
      & SERGUT_MMEMBER(data, timeMember3)
      & SERGUT_MMEMBER(data, intMember4)
      & SERGUT_MMEMBER(data, doubleMember5)
      & SERGUT_MMEMBER(data, floatMember6)
      & sergut::plainChild
      & SERGUT_MMEMBER(data, intMember7);
}

struct TestParent
{
public:
  bool operator==(const TestParent& rhs) const {
    return
        intMember1          == rhs.intMember1          &&
        intMember2          == rhs.intMember2          &&
        intMember3          == rhs.intMember3          &&
        childMember4        == rhs.childMember4        &&
        intMember5          == rhs.intMember5          &&
        intMember6          == rhs.intMember6          &&
        stringMember7       == rhs.stringMember7       &&
        charPtrMember8      == rhs.charPtrMember8      &&
        charMember9         == rhs.charMember9         &&
        childVectorMember10 == rhs.childVectorMember10 &&
        intVectorMember11   == rhs.intVectorMember11   &&
        childMember12       == rhs.childMember12;
  }
public:
  unsigned long long intMember1;
  unsigned long intMember2;
  unsigned int intMember3;
  TestChild childMember4;
  unsigned short intMember5;
  unsigned char intMember6;
  std::string stringMember7;
  std::string charPtrMember8;
  char charMember9;
  std::vector<TestChild2> childVectorMember10;
  std::vector<int> intVectorMember11;
  TestChild2 childMember12;
};

SERGUT_FUNCTION(TestParent, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, intMember1)
      & SERGUT_MMEMBER(data, intMember2)
      & sergut::children
      & SERGUT_MMEMBER(data, intMember3)
      & SERGUT_MMEMBER(data, childMember4)
      & SERGUT_NESTED_MMEMBER(data, intMember5, nestedIntMember5)
      & SERGUT_MMEMBER(data, intMember6)
      & SERGUT_MMEMBER(data, stringMember7)
      & SERGUT_MMEMBER(data, charPtrMember8)
      & SERGUT_MMEMBER(data, charMember9)
      & SERGUT_MMEMBER(data, childVectorMember10)
      & SERGUT_MMEMBER(data, intVectorMember11)
      & SERGUT_MMEMBER(data, childMember12);
}

struct Simple {
public:
  bool operator==(const Simple& rhs) const {
    return int1    == rhs.int1
        && double2 == rhs.double2
        && time3   == rhs.time3
        && char4   == rhs.char4
        && uchar5  == rhs.uchar5
        && time6   == rhs.time6;
  }
public:
  int int1;
  double double2;
  Time time3;
  char char4;
  unsigned char uchar5;
  Time time6;
};

SERGUT_FUNCTION(Simple, data, ar)
{
  ar
      & SERGUT_MMEMBER(data, int1)
      & SERGUT_MMEMBER(data, double2)
      & SERGUT_MMEMBER(data, time3)
      & sergut::children
      & SERGUT_NESTED_MMEMBER(data, char4, nestedChar4)
      & SERGUT_MMEMBER(data, uchar5)
      & SERGUT_NESTED_MMEMBER(data, time6, nestedTime6)
      ;
}
}
