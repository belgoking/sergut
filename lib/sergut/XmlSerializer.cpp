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

#include "XmlSerializer.h"

#include <map>

namespace sergut {

struct XmlSerializer::LevelStatus {

public:
  ValueType valueType = ValueType::Attribute;
};

struct XmlSerializer::Impl {
  Impl() { }
  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;

public:
  std::vector<LevelStatus> levelStatus;
  std::ostringstream out;
};



XmlSerializer::XmlSerializer()
  : impl(new Impl)
{
  impl->levelStatus.push_back(LevelStatus{});
  // The initial level allways starts up at Children-level
  impl->levelStatus.back().valueType = ValueType::Child;
}

XmlSerializer::XmlSerializer(const XmlSerializer &ref)
  : impl(ref.impl)
{
  impl->levelStatus.push_back(LevelStatus{});
}

XmlSerializer::~XmlSerializer() {
  impl->levelStatus.pop_back();
  if(impl->levelStatus.empty()) {
    delete impl;
  }
}

XmlSerializer &XmlSerializer::operator&(const ChildrenFollow &)
{
  assert(getValueType()==ValueType::Attribute);
  out() << ">";
  impl->levelStatus.back().valueType = ValueType::Child;
  return *this;
}

XmlSerializer &XmlSerializer::operator&(const PlainChildFollows &)
{
  assert(getValueType()==ValueType::Attribute);
  out() << ">";
  impl->levelStatus.back().valueType = ValueType::SingleChild;
  return *this;
}

std::string XmlSerializer::str() const
{
  return impl->out.str();
}

static
const std::map<char, std::string>& xmlEntities() {
  static const std::map<char, std::string> entities{
    {'"',  "&quot;"},
    {'&',  "&amp;" },
    {'\'', "&apos;"},
    {'<',  "&lt;"  },
    {'>',  "&gt;"  },
  };
  return entities;
}

void XmlSerializer::writeEscaped(const std::string& str)
{
  std::ostringstream& ostr = impl->out;
  std::string::const_iterator regionStartIt = str.begin();
  std::string::const_iterator regionEndIt = str.begin();
  const std::map<char, std::string>& entities = xmlEntities();
  while(regionEndIt != str.end()) {
    std::map<char, std::string>::const_iterator entityIt = entities.find(*regionEndIt);
    if(entityIt != entities.end()) {
      // found XML-Entity character
      if(regionStartIt != regionEndIt) {
        ostr.write(&*regionStartIt, regionEndIt-regionStartIt);
      }
      const std::string& entity = entityIt->second;
      ostr.write(entity.data(), entity.size());
      ++regionEndIt;
      regionStartIt = regionEndIt;
    } else {
      ++regionEndIt;
    }
  }
  if(regionEndIt != regionStartIt) {
    ostr.write(&*regionStartIt, regionEndIt-regionStartIt);
  }
}

XmlSerializer::ValueType XmlSerializer::getValueType() const
{
  return impl->levelStatus.back().valueType;
}

std::ostringstream &XmlSerializer::out()
{
  return impl->out;
}

} // namespace sergut
