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

#include "sergut/JsonSerializerBase.h"

#include "sergut/Misc.h"

#include <iomanip>
#include <map>
#include <vector>

namespace sergut {
namespace detail {


struct JsonSerializerBase::LevelStatus {
  bool firstOfLevel = true;
};

struct JsonSerializerBase::Impl {
  std::vector<LevelStatus> levelStatus;
  std::ostringstream out;
  uint8_t flags = static_cast<uint8_t>(Flags::None);
};



JsonSerializerBase::JsonSerializerBase(const Flags flags)
  : impl(new Impl)
{
  impl->flags = static_cast<uint8_t>(flags);
  impl->levelStatus.push_back(LevelStatus{});
}

JsonSerializerBase::JsonSerializerBase(const JsonSerializerBase&ref)
  : impl(ref.impl)
{
  impl->levelStatus.push_back(LevelStatus{});
}

JsonSerializerBase::~JsonSerializerBase() {
  impl->levelStatus.pop_back();
  if(impl->levelStatus.empty()) {
    delete impl;
  }
}

std::string JsonSerializerBase::str() const
{
  return impl->out.str();
}

static
const std::map<char, std::string>& specialCharacters() {
  static const std::map<char, std::string> entities{
    {'"' ,  "\\\"" },
    {'\\',  "\\\\" },
    {'\b',  "\\b"  },
    {'\f',  "\\f"  },
    {'\n',  "\\n"  },
    {'\r',  "\\r"  },
    {'\t',  "\\t"  },
  };
  return entities;
}

static
bool isPrintable(const char c) {
  const unsigned char ci = static_cast<unsigned char>(c);
  const unsigned char limit = 0x20;
  return ci >= limit;
}

void JsonSerializerBase::writeEscaped(const std::string &str)
{
  std::ostringstream& ostr = impl->out;
  std::string::const_iterator regionStartIt = str.begin();
  std::string::const_iterator regionEndIt = str.begin();
  const std::map<char, std::string>& entities = specialCharacters();
  while(regionEndIt != str.end()) {
    std::map<char, std::string>::const_iterator entityIt = entities.find(*regionEndIt);
    const bool isNonPrintable = !isPrintable(*regionEndIt);
    if(entityIt != entities.end() || isNonPrintable) {
      // found XML-Entity character
      if(regionStartIt != regionEndIt) {
        ostr.write(&*regionStartIt, regionEndIt-regionStartIt);
      }
      if(entityIt != entities.end()) {
        const std::string& entity = entityIt->second;
        ostr.write(entity.data(), entity.size());
      } else {
        // other non-printable
        StreamFormatBackup formatGuard(ostr);
        ostr << "\\u" << std::setfill('0') << std::setw(4) << std::hex << (unsigned int)*regionEndIt;
      }
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

void JsonSerializerBase::addCommaIfNeeded()
{
  bool& firstOfLevel = impl->levelStatus.back().firstOfLevel;
  if(firstOfLevel) {
    firstOfLevel = false;
  } else {
    impl->out << ",";
  }
}

std::ostream &JsonSerializerBase::out()
{
  return impl->out;
}

bool JsonSerializerBase::hasFlag(detail::JsonSerializerBase::Flags flag) const {
  return (impl->flags & static_cast<uint8_t>(flag)) != 0;
}

} // namespace detail
} // namespace sergut
