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

#include "sergut/ParsingException.h"
#include "sergut/SerializerBase.h"
#include "sergut/Util.h"
#include "sergut/detail/DummySerializer.h"
#include "sergut/detail/MemberDeserializer.h"
#include "sergut/detail/XmlDeserializerHelper.h"
#include "sergut/xml/PullParser.h"

#include <string>
#include <vector>
#include <list>
#include <set>
#include <cassert>
#include <iostream>

/**
 * This data driven XmlDeserializer pulls XML events out of a PullParser and tries
 * to match them to the data type that it is requested to deserialize into.
 *
 * For structured data types (classes or structs) with exception of those that are
 * deserialized individually from string, the XmlDeserializer works in two
 * steps for each node:
 * \ol In the first step it retrieves all the members of the data type and keeps
 *     a deserializer for this member in a map keyed by the attribute or tag name.
 *     The member handlers are constructed using the \c MemberDeserializer, which
 *     creates a virtual function that calls the appropriat
 *     \c XmlDeserializer::handleChild function. Those virtual functions are then
 *     stored in a \c map for latter use.
 * \ol In the second step the elements of the current nesting level are pulled out
 *     of the \c PullParser. In case there is a matching handler for that XML-Node,
 *     the handler is executed with the PullParser as parameter.
 */
namespace sergut {


class XmlDeserializer
{
  template<typename T, typename S> friend class detail::MemberDeserializer;
  typedef detail::MemberDeserializer<XmlDeserializer, xml::PullParser&> MyMemberDeserializer;
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
  XmlDeserializer(const char* xml) : XmlDeserializer(sergut::misc::ConstStringRef(xml)) { }
  XmlDeserializer(const std::string& xml) : XmlDeserializer(sergut::misc::ConstStringRef(xml)) { }
  XmlDeserializer(const misc::ConstStringRef& xml);

//  /// Initial call to the serializer
//  /// \param name The name of the outer tag
//  /// \deprecated Do not use this function, use the one without an xmlValueType instead.
//  template<typename DT>
//  DT deserializeData(const char* name, const sergut::XmlValueType xmlValueType) {
//    static_assert(!XmlDeserializerHelper::canDeserializeIntoAttribute<DT>(),
//                  "Deserializing using this function only makes sense for non CDATA types, as CDATA types are always serialized as PlainChild");
//    // xmlValueType can be something other than XmlValueType::Attribute only if DT is string serializable or a basic data type
//    assert(xmlValueType == XmlValueType::Attribute);
//    DT data;
//    doDeserializeData(MyMemberDeserializer::toNamedMember(name, data, true), xmlValueType);
//    return data;
//  }

  /// Initial call to the serializer
  /// \param name The name of the outer tag
  template<typename DT>
  DT deserializeData(const char* name) {
    DT data;
    if(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<DT>()) {
      // In this case we don't call any serialize()-function. Thus this has
      // to be serialized to something like '<name>value</name>'.
      doDeserializeData(MyMemberDeserializer::toNamedMember(
                          name,
                          MyMemberDeserializer::toNestedMember("DUMMY", data, true, XmlValueType::SingleChild),
                          true));
    } else {
      // In this case the serialize()-function is called and we don't have to
      // do any special handling.
      doDeserializeData(MyMemberDeserializer::toNamedMember(name, data, true));
    }
    return data;
  }

  template<typename DT, XmlValueType xmlValueType = XmlValueType::Child>
  DT deserializeNestedData(const char* outerName, const char* innerName) {
    static_assert(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<DT>()
                  || xmlValueType == XmlValueType::Child,
                  "Datatypes that cannot be serialized as an Attribute (i.e. those for which serialize() is called), "
                  "must be deserialized with xmlValueType == sergut::XmlValueType::Child.");
    DT data;
    doDeserializeData(MyMemberDeserializer::toNamedMember(outerName,
                                                          MyMemberDeserializer::toNestedMember(innerName, data, true,
                                                                                              xmlValueType),
                                                          true));
    return data;
  }

  template<typename DT>
  static DT deserializeFromSnippet(const char* name, xml::PullParser& currentXmlNode) {
    DT data;
    XmlDeserializer deser(currentXmlNode);
    if(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<DT>()) {
      // In this case we don't call any serialize()-function. Thus this has
      // to be serialized to something like '<name>value</name>'.
      deser.doDeserializeFromSnippet(
            MyMemberDeserializer::toNamedMember(name,
                                                MyMemberDeserializer::toNestedMember("DUMMY", data, true, XmlValueType::SingleChild),
                                                true));
    } else {
      // In this case the serialize()-function is called and we don't have to
      // do any special handling.
      deser.doDeserializeFromSnippet(MyMemberDeserializer::toNamedMember(name, data, true));
    }
    return data;
  }

  template<typename DT, XmlValueType xmlValueType = XmlValueType::Child>
  static DT deserializeNestedFromSnippet(const char* outerName,
                                  const char* innerName,
                                  xml::PullParser& currentXmlNode)
  {
    static_assert(detail::XmlDeserializerHelper::canDeserializeIntoAttribute<DT>()
                  || xmlValueType == XmlValueType::Child,
                  "Datatypes that cannot be serialized as an Attribute (i.e. those for which serialize() is called), "
                  "must be deserialized with xmlValueType == sergut::XmlValueType::Child.");
    DT data;
    XmlDeserializer deser(currentXmlNode);
    deser.doDeserializeFromSnippet(
          MyMemberDeserializer::toNamedMember(outerName,
                                              MyMemberDeserializer::toNestedMember(innerName, data, true,
                                                                                   xmlValueType),
                                              true));
    return data;
  }

private:
  template<typename DT>
  void doDeserializeData(const NamedMemberForDeserialization<DT>& data)
  {
    if(xmlDocument->parseNext() != xml::ParseTokenType::OpenDocument) {
      throw ParsingException("Invalid XML-Document", ErrorContext(*xmlDocument));
    }
    xmlDocument->parseNext();
    doDeserializeFromSnippet(data);
  }

  template<typename DT>
  void doDeserializeFromSnippet(const NamedMemberForDeserialization<DT>& data)
  {
    if(xmlDocument->getCurrentTokenType() != xml::ParseTokenType::OpenTag) {
      throw ParsingException("Invalid XML-Document", ErrorContext(*xmlDocument));
    }
    if(data.name != nullptr && xmlDocument->getCurrentTagName() != data.name) {
      throw ParsingException("Wrong opening Tag in XML-Document", ErrorContext(*xmlDocument));
    }
    handleChild(data, XmlValueType::Child, *xmlDocument);
  }

private:
  XmlDeserializer(xml::PullParser& currentXmlNode);

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

  static void handleChild(const NamedMemberForDeserialization<bool>& data, const XmlValueType valueType, xml::PullParser& state);

  static void handleChild(const NamedMemberForDeserialization<double>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<float>& data, const XmlValueType valueType, xml::PullParser& state);

  // String types
  static void handleChild(const NamedMemberForDeserialization<std::string>& data, const XmlValueType valueType, xml::PullParser& state);
  static void handleChild(const NamedMemberForDeserialization<char>& data, const XmlValueType valueType, xml::PullParser& state);

  static void handleChild(const NamedMemberForDeserialization<char*>& data, const XmlValueType valueType, xml::PullParser& state) = delete;


  // Containers as members
  template<typename DT>
  static void handleChild(const NamedMemberForDeserialization<std::list<DT>>& data, const XmlValueType valueType, xml::PullParser& state) {
    while(checkNextContainerElement(data.name, valueType, state)) {
      DT element;
      handleChild(NamedMemberForDeserialization<DT>(data.name, element, true), valueType, state);
      data.data.push_back(element);
      if(state.getCurrentTokenType() == xml::ParseTokenType::Text) {
        state.parseNext();
      }
    }
  }

  template<typename DT>
  static void handleChild(const NamedMemberForDeserialization<std::set<DT>>& data, const XmlValueType valueType, xml::PullParser& state) {
    while(checkNextContainerElement(data.name, valueType, state)) {
      DT element;
      handleChild(NamedMemberForDeserialization<DT>(data.name, element, true), valueType, state);
      data.data.insert(element);
      if(state.getCurrentTokenType() == xml::ParseTokenType::Text) {
        state.parseNext();
      }
    }
  }

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

  /**
   * SFINAE trick to ensure that this template function is called only if SERGUT_FUNCTION(DT, data, ar) exists for datatype DT.
   * \see handleChild that is responsible for SERGUT_DESERIALIZE_FROM_STRING(DT, variableName, stringVariableName)
   */
  // structured data
  template<typename DT>
  static auto handleChild(const NamedMemberForDeserialization<DT>& data, const XmlValueType valueType, xml::PullParser& state)
  -> decltype(serialize(detail::DummySerializer::dummyInstance(), data.data, static_cast<typename std::decay<DT>::type*>(nullptr)),void())
  {
    assert(state.getCurrentTokenType() == xml::ParseTokenType::OpenTag);
    if(valueType != XmlValueType::Child) {
      throw new ParsingException("Expected XmlValueType::Child when deserializing members", ErrorContext(state));
    }
    // first descend to members
    state.parseNext();

    MyMemberDeserializer memberDeserializer(true);
    serialize(memberDeserializer, data.data, static_cast<typename std::decay<DT>::type*>(nullptr));
    feedMembers(memberDeserializer, state);
  }

  /**
   * SFINAE trick to ensure that this template function is called only if SERGUT_DESERIALIZE_FROM_STRING(DT, variableName, stringVariableName)
   * exists for datatype DT.
   * \see handleChild that is responsible for SERGUT_FUNCTION(DT, data, ar)
   */
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

private:
  std::unique_ptr<xml::PullParser> ownXmlDocument;
  xml::PullParser* xmlDocument;
};

} // namespace sergut
