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
#include "sergut/xml/PullParser.h"

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
  class ErrorContext: public ParsingException::ErrorContext {
  public:
    ErrorContext(const xml::PullParser& pParser) : parser(&pParser) { }
    std::size_t getRow() const override;
    std::size_t getColumn() const override;

  private:
    const xml::PullParser* parser;
  };

public:
  XmlDeserializer(const std::string& xml);
  ~XmlDeserializer();

  /// Initial call to the serializer
  /// \param name The name of the outer tag
  template<typename DT>
  DT deserializeData(const char* name) {
    DT data;
    doDeserializeData(MyMemberDeserializer::toNamedMember(name, data, true), XmlValueType::Attribute);
    return data;
  }

  template<typename DT>
  DT deserializeNestedData(const char* outerName, const char* innerName) {
    DT data;
    doDeserializeData(MyMemberDeserializer::toNamedMember(outerName, MyMemberDeserializer::toNamedMember(innerName, data, true), true), XmlValueType::Child);
    return data;
  }

private:
  template<typename DT>
  void doDeserializeData(const NamedMemberForDeserialization<DT>& data, const XmlValueType valueType)
  {
    xml::PullParser& pullParser = getPullParser();
    if(pullParser.parseNext() != xml::ParseTokenType::OpenDocument) {
      throw ParsingException("Invalid XML-Document", ErrorContext(pullParser));
    }
    if(pullParser.parseNext() != xml::ParseTokenType::OpenTag) {
      throw ParsingException("Invalid XML-Document", ErrorContext(pullParser));
    }
    if(data.name != nullptr && pullParser.getCurrentTagName() != data.name) {
      throw ParsingException("Wrong opening Tag in XML-Document", ErrorContext(pullParser));
    }
    handleChild(data, valueType, pullParser);
  }

private:
  // the following functions are called by MemberDeserializer

  // Signed integers
  static void handleChild(const NamedMemberForDeserialization<long long>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<long>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<int>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<short>& data, const XmlValueType valueType, xml::PullParser& state);

  // Unsigned integers
  static void handleChild(const NamedMemberForDeserialization<unsigned long long>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<unsigned long>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<unsigned int>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<unsigned short>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<unsigned char>& data, const XmlValueType valueType, xml::PullParser& state);

  static void handleChild(const NamedMemberForDeserialization<double>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<float>& data, const XmlValueType valueType, xml::PullParser& state);

  // String types
  static void handleChild(const NamedMemberForDeserialization<std::string>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<char>& data, const XmlValueType valueType, xml::PullParser& state);

  static void handleChild(const NamedMemberForDeserialization<char*>& data, const XmlValueType valueType, xml::PullParser& state) = delete;


  // Containers as members
  template<typename DT>
  static void handleChild(const NamedMemberForDeserialization<std::vector<DT>>& data, const XmlValueType valueType, xml::PullParser& state) {
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
  static auto handleChild(const NamedMemberForDeserialization<DT>& data, const XmlValueType valueType, xml::PullParser& state)
  -> decltype(serialize(DummySerializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)),void())
  {
    (void)valueType;
    MyMemberDeserializer memberDeserializer(true);
    serialize(memberDeserializer, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
    feedMembers(memberDeserializer, state);
  }

  template<typename DT>
  static auto handleChild(const NamedMemberForDeserialization<DT>& data, const XmlValueType valueType, xml::PullParser& state)
  -> decltype(deserializeFromString(data.data, std::string()),void())
  {
    deserializeFromString(data.data, popString(valueType, state));
  }

private:
  static void feedMembers(MyMemberDeserializer& retriever, xml::PullParser& state);
  static std::string popString(const XmlValueType valueType, xml::PullParser& state);
  static bool checkNextContainerElement(const char* name, const XmlValueType valueType, xml::PullParser& state);
  xml::PullParser& getPullParser();
private:
  Impl* impl = nullptr;
};

} // namespace sergut
