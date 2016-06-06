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

#include "XmlDeserializerTiny.h"

#include "sergut/ParsingException.h"

#include <map>
#include <sstream>

namespace sergut {

XmlDeserializerTiny::XmlDeserializerTiny(const std::string& xml)
  : parentValueType(ValueType::Child)
  , valueType(ValueType::Attribute)
  , currentElement(nullptr)
{
  xmlDocument.reset(new TiXmlDocument);
  if(xmlDocument->Parse(xml.c_str()) != nullptr) {
    throw ParsingException(std::string("Error parsing XML"));
  }
  currentElement = xmlDocument->RootElement();
}

XmlDeserializerTiny::XmlDeserializerTiny(const XmlDeserializerTiny& ref)
  : parentValueType(ref.valueType)
  , valueType(ValueType::Attribute)
  , currentElement(ref.currentElement)
{ }

XmlDeserializerTiny::~XmlDeserializerTiny() { }

void XmlDeserializerTiny::doReadInto(const char* str, unsigned char& data) {
  unsigned short tmp;
  std::istringstream(str) >> tmp;
  if(tmp > 0xFF) {
    throw ParsingException("unsigned char value is out of range");
  }
  data = tmp;
}

XmlDeserializerTiny &XmlDeserializerTiny::operator&(const ChildrenFollow &)
{
  assert(valueType == ValueType::Attribute);
  valueType = ValueType::Child;
  return *this;
}

XmlDeserializerTiny &XmlDeserializerTiny::operator&(const PlainChildFollows &)
{
  assert(valueType==ValueType::Attribute);
  valueType = ValueType::SingleChild;
  return *this;
}

} // namespace sergut
