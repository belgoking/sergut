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

#include "sergut/SerializerBase.h"
#include "sergut/MemberDeserializer.h"
#include "sergut/ParsingException.h"
#include "sergut/Util.h"
#include "xml/PullParser.h"

#include <string>
#include <vector>
#include <list>
#include <set>
#include <cassert>
#include <iostream>

/*
 * <foo>
 *   <bar baz="1">val</bar>
 *   <bam>val</bam>
 *   <bo><tox>tom</tox></bo>
 *   <mer><fo1>1</fo1><fo1>2</fo1></mer>
 *   <collection><member>1</member><member>2</member></collection>
 * <!-- ^-from 'foo' ^-from? -->
 *   <huk foo="fufu"/>
 * </foo>
 */
namespace sergut {

class XmlDeserializer
{
  template<typename T, typename S> friend class MemberDeserializer;
  typedef MemberDeserializer<XmlDeserializer, xml::PullParser&> MyMemberDeserializer;
  struct Impl;
public:
  XmlDeserializer(const std::string& xml);
  ~XmlDeserializer();

  /// Initial call to the serializer
  /// \param name The name of the outer tag
  template<typename DT, typename NAMED_MEMBER_FOR_DESERIALIZATION = NamedMemberForDeserialization<DT>>
  DT deserializeData(const char* name, const ValueType valueType) {
    DT data;
    xml::PullParser& pullParser = getPullParser();
    if(pullParser.parseNext() != xml::ParseTokenType::OpenDocument) {
      throw ParsingException("Invalid XML-Document");
    }
    if(pullParser.parseNext() != xml::ParseTokenType::OpenTag) {
      throw ParsingException("Invalid XML-Document");
    }
    if(name != nullptr && pullParser.getCurrentTagName() != name) {
      throw ParsingException("Wrong opening Tag in XML-Document");
    }
//    tinyxml2::XMLNode* currentNode = &getCheckedXmlRootNode(name);
    handleChild(NAMED_MEMBER_FOR_DESERIALIZATION(name, data, true), valueType, pullParser);
    return data;
  }

private:
  // the following functions are called by MemberDeserializer

  // Signed integers
  static void handleChild(const NamedMemberForDeserialization<long long>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<long>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<int>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<short>& data, const ValueType valueType, xml::PullParser& state);

  // Unsigned integers
  static void handleChild(const NamedMemberForDeserialization<unsigned long long>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<unsigned long>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<unsigned int>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<unsigned short>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<unsigned char>& data, const ValueType valueType, xml::PullParser& state);

  static void handleChild(const NamedMemberForDeserialization<double>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<float>& data, const ValueType valueType, xml::PullParser& state);

  // String types
  static void handleChild(const NamedMemberForDeserialization<std::string>& data, const ValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<char>& data, const ValueType valueType, xml::PullParser& state);

  static void handleChild(const NamedMemberForDeserialization<char*>& data, const ValueType valueType, xml::PullParser& state) = delete;


  // Containers as members
  template<typename DT>
  static void handleChild(const NamedMemberForDeserialization<std::vector<DT>>& data, const ValueType valueType, xml::PullParser& state) {
    while(checkNextContainerElement(data.name, valueType, state)) {
      DT element;
      handleChild(NamedMemberForDeserialization<DT>(data.name, element, true), valueType, state);
      data.data.push_back(element);
      if(state.getCurrentTokenType() == xml::ParseTokenType::Text) {
        state.parseNext();
      }
    }
  }

  // structured data
  template<typename DT>
  static auto handleChild(const NamedMemberForDeserialization<DT>& data, const ValueType valueType, xml::PullParser& state)
  -> decltype(serialize(DummySerializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)),void())
  {
    (void)valueType;
    MyMemberDeserializer memberDeserializer(true);
    serialize(memberDeserializer, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
    feedMembers(memberDeserializer, state);
  }

  template<typename DT>
  static auto handleChild(const NamedMemberForDeserialization<DT>& data, const ValueType valueType, xml::PullParser& state)
  -> decltype(deserializeFromString(data.data, std::string()),void())
  {
    deserializeFromString(data.data, popString(valueType, state));
  }

private:
  static void feedMembers(MyMemberDeserializer& retriever, xml::PullParser& state);
  static std::string popString(const ValueType valueType, xml::PullParser& state);
  static bool checkNextContainerElement(const char* name, const ValueType valueType, xml::PullParser& state);
  xml::PullParser& getPullParser();
private:
  Impl* impl = nullptr;
};

} // namespace sergut
