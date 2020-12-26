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

#include "sergut/DeserializerBase.h"

#include "sergut/XmlValueType.h"

#include <list>
#include <memory>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace sergut {
namespace detail {

class MemberDeserializerBase: public DeserializerBase {
public:
  static const std::string SINGLE_CHILD;
};

template<typename SERIALIZER, typename SERIALIZATION_STATE>
class MemberDeserializer: public MemberDeserializerBase {
public:
  typedef SERIALIZER SerializerType;
  typedef SERIALIZATION_STATE SerializationState;
  struct HolderBase {
    HolderBase(const XmlValueType pValueType) : valueType(pValueType) { }
    HolderBase(const HolderBase& ref) = delete;
    HolderBase& operator=(const HolderBase& ref) = delete;
    virtual ~HolderBase() { }
    virtual void execute(SerializationState deser) = 0;
    virtual bool isMandatory() const = 0;
    virtual bool isContainer() const = 0;
    virtual const char* getName() const = 0;
  protected:
    const XmlValueType valueType;
  };
  typedef std::map<std::string, std::shared_ptr<HolderBase>> Members;

  template<typename WrappedDT>
  struct Holder: public HolderBase {
    Holder(const WrappedDT& pWrappedDT, const XmlValueType pValueType)
      : HolderBase(pValueType), wrappedDT(pWrappedDT)
    { }
    void execute(SerializationState state) override {
      SerializerType::handleChild(wrappedDT, HolderBase::valueType, state);
    }
    bool isMandatory() const override {
      return wrappedDT.mandatory;
    }
    bool isContainer() const override {
      return isContainer(static_cast<typename WrappedDT::value_type*>(nullptr));
    }
    virtual const char* getName() const override {
      return wrappedDT.name;
    }
  private:
    template<typename T>
    static bool isContainer(T*) { return false; }
    template<typename InnerT>
    static bool isContainer(std::vector<InnerT>*) { return true; }
    template<typename InnerT>
    static bool isContainer(std::list<InnerT>*) { return true; }
    template<typename InnerT>
    static bool isContainer(std::set<InnerT>*) { return true; }
  public:
    WrappedDT wrappedDT;
  };

public:
  MemberDeserializer(const bool pSingleChildSupported) : singleChildSupported(pSingleChildSupported) { }

  /// Members until this marker are rendered as XML-Attributes, after it as sub-elements
  MemberDeserializer& operator&(const ChildrenFollow&)
  {
    valueType = XmlValueType::Child;
    return *this;
  }

  /// Members until this marker are rendered as XML-Attributes,
  /// After this marker there should only be one member left, that must be
  /// renderable as a simple XML-Type (i.e. a number or a string)
  MemberDeserializer& operator&(const PlainChildFollows&)
  {
    valueType = XmlValueType::SingleChild;
    return *this;
  }

  template<typename WrappedDT>
  MemberDeserializer& operator&(WrappedDT const& data) {
    if(singleChildSupported && valueType == XmlValueType::SingleChild) {
      members[SINGLE_CHILD] = std::make_shared<Holder<WrappedDT>>(data, valueType);
    } else {
      members[data.name] = std::make_shared<Holder<WrappedDT>>(data, valueType);
    }
    return *this;
  }

  XmlValueType getValueType() const { return valueType; }
  std::shared_ptr<HolderBase> popMember(const std::string& memberName) {
    std::shared_ptr<HolderBase> ret;
    typename Members::const_iterator membersIt = members.find(memberName);
    if(membersIt != members.end()) {
      ret = membersIt->second;
      members.erase(membersIt);
    }
    return ret;
  }

  const std::map<std::string, std::shared_ptr<HolderBase>>& getMembers() const { return members; }

private:
  const bool singleChildSupported = true;
  Members members;
  XmlValueType valueType = XmlValueType::Attribute;
};


} // namespace detail
} // namespace sergut
