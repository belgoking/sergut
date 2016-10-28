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

#include "sergut/TestSupportClasses.h"

class MyInterface {
public:
  virtual ~MyInterface() { }

  virtual
  SomeMoreComplexTestData
  constructSomeMoreComplexTestData(std::uint8_t hour1, const std::uint8_t minute1, const std::uint8_t& second1,
                                   char someLetter, std::uint16_t someUnsignedShortInt, const Time& time2) const = 0;
  virtual
  uint32_t sumUpSomeData(std::uint32_t someUInt, const Time& t, uint32_t otherUInt) const = 0;

  virtual
  uint32_t empty() const = 0;

  template<typename Server>
  void initialize(Server& server) {
    server.add(std::string("constructSomeMoreComplexTestData"), this, std::string("returnType"),
               &MyInterface::constructSomeMoreComplexTestData,
               typename Server::Parameter("hour1"),
               typename Server::Parameter("minute1"),
               typename Server::Parameter("second1"),
               typename Server::Parameter("someLetter"),
               typename Server::Parameter("someUnsignedShortInt"),
               typename Server::Input("time2")
               );
    server.add("sumUpSomeData", this, "returnUInt32",
               &MyInterface::sumUpSomeData,
               typename Server::Parameter("someUInt"),
               typename Server::Input("t"),
               typename Server::Parameter("otherUInt")
               );
    server.add("empty", this, "rt", &MyInterface::empty);
  }
};
