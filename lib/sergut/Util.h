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

/** \mainpage Serialization Library for C++ (mainly XML)
 *
 * \section introduction Introduction
 * The \c sergut library provides an interface similar to \c boost-serialization
 * that permits for out-of-class serialization/deserialization of nested C++
 * classes in XML. A design goal of this library was to be able to easily be able
 * to create a matching between a given XML and C++ datastructures. Something that
 * is possible in JAXB for Java but that is lacking in all C++ serialization
 * libraries that I have looked at before I started this library. On the other
 * hand I wanted to avoid the need of creating marker classes as is needed in JAXB,
 * just to be able to create different constructs, such as lists.
 *
 * \subsection notation Notation and Conventions
 * \c sergut is for serialization and deserialization and several functions can
 * be used for both of these tasks. However, to keep the text readable I will
 * just refer to the serialization exemplary for both. So instead of writing
 * 'serialize/deserialize' everywhere, I will just be writing 'serialize'
 * when I deem it to be reasonably clear that both are meant.
 *
 * \section simpleClasses Serializing Simple Classes
 * As a first example let's regard the following classes:
   \verbatim
    1  #include <vector>
    2  struct InnerExample {
    3    int intMember;
    4    std::string stringMember;
    5  };
    6  struct OuterExample {
    7    std::string stringMember;
    8    InnerExample innerExampleMember;
    9  };
   \endverbatim
 * They should be serialized into something like:
   \verbatim
   <outerExample stringMember="some content">
      <innerExampleMember intMember="23">
        <stringMember>Blah</stringMember>
      </innerExampleMember>
   </outerExample>
   \endverbatim
 *
 * To achieve this in \c sergut you have to create a
 * serialization/deserialization function for each of the classes, as so:
   \verbatim
   10  SERGUT_FUNCTION(InnerExample, data, ar) {
   11    ar & SERGUT_MMEMBER(data, intMember)
   12       & sergut::children
   13       & SERGUT_MMEMBER(data, stringMember);
   14  }
   15  SERGUT_FUNCTION(OuterExample, data, ar) {
   16    ar & SERGUT_MMEMBER(data, stringMember)
   17       & sergut::children
   18       & SERGUT_MMEMBER(data, innerExampleMember);
   19  }
   \endverbatim
 * In each of those functions, it is specified, how the members of the classes
 * are to be serialized. Let's have a look at the serialization function of
 * \c OuterExample:
 * \li Line 16 specifies the function signature. It says that the datatype that
 *     is to be serialized is \c OuterExample, that within the function the
 *     instance of \c OuterExample is stored in a variable with the name
 *     \c data. And that the serializer that will be used to serialize the
 *     instance of \c OuterExample is referred to as \c ar.
 * \li In Line 17 the first member of \c OuterExample is serialized. By default
 *     members are serialized as XML-Attributes. Obviously, only simple
 *     datatypes or datatypes that are serialized as strings can be set as
 *     XML-Attributes. If you try to serialize some class as an attribute you
 *     will get an exception at runtime.
 * \li In Line 18 it is specified, that all following members will be
 *     serialized as children. Simple datatypes will be serialized to someting
 *     similar to
       \verbatim
       <memberName>Value</memberName>
       \endverbatim
 * \li In Line 19 innerExampleMember is serialized. This is done by calling its
 *     \c SERGUT_FUNCTION.
 * This is how arbitrarily complex datastructures can be serialized using
 * \c sergut.
 *
 * For convenience and to reduce the risk of Errors, \c sergut provides the
 * macros \c SERGUT_FUNCTION(), \c SERGUT_MMEMBER() and others. In some rare
 * situations you will be required to write the code manually. However, you
 * have to be cautious, as there are some aspects that have to be regarded.
 * Those will be described later (TBD).
 *
 * \section sequences Serializing Sequences
 * Data of the type \c std::vector, \c std::list, and \c std::set is serialized
 * as repeating XML with the same outer tag, e.g.
   \verbatim
   <tagName>1</tagName>
   <tagName>2</tagName>
       ...
   <tagName>N</tagName>
   \endverbatim
 * This happens automatically, when you serialize one of those collections
 * (either as member of a class or directly). However, often XML-specifications
 * require an additional nesting level, as in
   \verbatim
   <outerTag>
     <tagName>1</tagName>
     <tagName>2</tagName>
         ...
     <tagName>N</tagName>
   </outerTag>
   \endverbatim
 * This can be realized by creating a dummy class, as in
   \verbatim
    1 struct OuterTag {
    2   std::vector<int> tagName;
    3 };
    4 struct MyClass1 {
    5   OuterTag outerTag;
    6 };
    7 SERGUT_FUNCTION(OuterTag, data, ar) {
    8   ar & sergut::children
    9      & SERGUT_MMEMBER(data, tagName);
   10 }
   11 SERGUT_FUNCTION(MyClass1, data, ar) {
   12   ar & sergut::children
   13      & SERGUT_MMEMBER(data, outerTag);
   14 }
   \endverbatim
 * To prevent you of having to create that dummy class in you datastructure,
 * \c sergut provides the helping macro \c SERGUT_NESTED_MMEMBER(). Using that
 * the previous 14 lines get reduced to the following 7 lines:
   \verbatim
    1 struct MyClass2 {
    2   OuterTag outerTag;
    3 };
    4 SERGUT_FUNCTION(MyClass2, data, ar) {
    5   ar & sergut::children
    6      & SERGUT_NESTED_MMEMBER(data, outerTag, tagName);
    7 }
   \endverbatim
 *
 * \section plainChild Plain Child
 * The code that we have seen so far does not permit the serialization of the
 * following:
   \verbatim
   <tagName attribute="att value">plain data</tagName>
   \endverbatim
 * Instead one always has to add some (possibly unwanted) XML-nesting level as
 * shown in the following example:
   \verbatim
   <tagName attribute="att value"><unwantedTag>plain data</unwantedTag></tagName>
   \endverbatim
 * \c sergut provides the marker variable \c sergut::plainChild that entitles
 * you to omit the tag \c unwantedTag.
   \verbatim
    1 struct MyClass3 {
    2   std::string attribute;
    3   std::string plainChildMember;
    4 };
    5 SERGUT_FUNCTION(MyClass3, data, ar) {
    6   ar & SERGUT_MMEMBER(data, attribute)
    7      & sergut::plainChild
    8      & SERGUT_MMEMBER(data, plainChildMember);
    9 }
   \endverbatim
 * Note that the member name \c plainChildMember does not appear in the XML.
 * \section fromToString Serialize to and Deserialize from String
 * Todo
 *
 * \section serialize Serialization
 * To serialize a data structure (that has its \c SERGUT_FUNCTION) you require
 * a serializer.
 *
 * \subsection serializeXml XML
 * One such serializer is the \c sergut::XmlSerializer. It is used as in the
 * following example. Note that we use the classes that were created in the
 * previous sections.
   \verbatim
    1 const OuterExample data = createOuterExample();
    2 sergut::XmlSerializer ser;
    3 ser.serializeData("outerTag", data);
    4 const std::string result = ser.str();
   \endverbatim
 * Note that the outer tag of the XML has to be given to the function. This is
 * as the serialization usually uses the member name as tag name. However,
 * the class instance given to \c serializeData() is noones member, so we have
 * to tell what the outest tag should be called.
 *
 * Note also that serialized content is always encoded in \c UTF-8.
 *
 * \subsection serializeJson JSON
 * Serialization to \c JSON is similar to \c XML serialization. However, as in
 * \c JSON the outest tag does not have a name, the outer tag name is not
 * needed.
   \verbatim
    1 const OuterExample data = createOuterExample();
    2 sergut::JsonSerializer ser;
    3 ser.serializeData(data);
    4 const std::string result = ser.str();
   \endverbatim
 *
 * \section deserialize Deserialization (XML)
 * There are several deserializers for XML, depending on what you need. The
 * options are:
 * \li \c sergut::XmlDeserializer A deserializer based on an XML-PullParser. This has
 *     the advantage that it can to in-place parsing and that it can work with
 *     partial XML. If you are looking for speed this also is an option, but
 *     \c sergut::XmlDeserializerTiny2 is even faster (and much simpler).
 * \li \c sergut::XmlDeserializerTiny A deserializer based on the tinyxml
 *     library version 1. Don't use this unless you are already using that
 *     version of the library in your project. It is slow and memory intensive.
 * \li \c sergut::XmlDeserializerTiny2 A deserializer based on the tinyxml
 *     library version 2. If you don't have memory constraints, don't rely
 *     on partial XML handling, and don't mind having another library
 *     dependency, you should use this.
 *
 * \subsection deserializeXmlDeserializer XmlDeserializer
 * The pull parser based deserializer is used as follows in the normal case:
   \verbatim
    1 sergut::XmlDeserializer ser(xml);
    2 const OuterExample data = cser.deserializeData<OuterExample>("outerTag");
   \endverbatim
 * The \c XML can be in UTF-8, UTF-16LE, or UTF-16BE.
 *
 * Note that (again) the outer tag of the XML has to be given to the function.
 *
 * Nested content is deserialized as follows:
   \verbatim
    1 sergut::XmlDeserializer ser(xml);
    2 const OuterExample data = cser.deserializeNestedData<OuterExample, sergut::XmlValueType::Child>("outerTag", "innerTag");
   \endverbatim
 *
 * To deserialize partial \c XML you have to manually instantiate the pull
 * parser, peel off the leading Tags that preceed the object that you want to
 * read out of the input data, and then user one of the fuctions
 * \c deserializeFromSnippet() or \c deserializeNestedFromSnippet() as in the
 * following example. Let's regard code that parses a large but simple XML of
 * the kind:
   \verbatim
   <elements>
     <element id="1"/>
     <element id="2"/>
     <element id="3"/>
        ...
     <element id="N"/>
   </elements>
   \endverbatim
  This could be parsed successively by the following code:
   \verbatim
    1 std::unique_ptr<sergut::xml::PullParser> pullParser
    2     = sergut::xml::PullParser::createParser(sergut::misc::ConstStringRef(xml));
    3
    4 if(pullParser->parseNext()         != sergut::xml::ParseTokenType::OpenDocument) throw Exception();
    5 if(pullParser->parseNext()         != sergut::xml::ParseTokenType::OpenTag)      throw Exception();
    6 if(pullParser->getCurrentTagName() != std::string("root"))                       throw Exception();
    7 if(pullParser->parseNext()         != sergut::xml::ParseTokenType::OpenTag)      throw Exception();
    8
    9 while(pullParser->getCurrentTokenType() == sergut::xml::ParseTokenType::OpenTag
   10    && pullParser->getCurrentTagName() == std::string("data")) {
   11   pullParser->setSavePointAtCurrentTag();
   12   try {
   13     handle(sergut::XmlDeserializer::deserializeNestedFromSnippet<int, sergut::XmlValueType::Attribute>(
   14              "element", "id", *pullParser)) == i);
   15   } catch(const sergut::ParsingException& e) {
   16       pullParser->restoreToSavePoint();
   17       const std::string continuationOfXml = getMoreXml();
   18       pullParser->appendData(continuationOfXml.c_str(), continuationOfXml.size());
   19   }
   20 }
   21 if(pullParser->getCurrentTokenType() != sergut::xml::ParseTokenType::CloseTag) throw Exception();
   22 if(pullParser->getCurrentTagName() != std::string("root"))                     throw Exception();
   \endverbatim
 *
 * \subsection deserializeXmlDeserializerTiny XmlDeserializerTiny
 * This deserializer is used as follows:
   \verbatim
    1 sergut::XmlDeserializerTiny ser(xml);
    2 const OuterExample data = cser.deserializeData<OuterExample>("outerTag");
   \endverbatim
 * AFAIK the only supported encoding for tinyxml is \c UTF-8.
 *
 * Note that (again) the outer tag of the XML has to be given to the function.
 *
 * Nested content is deserialized as follows:
   \verbatim
    1 sergut::XmlDeserializerTiny ser(xml);
    2 const OuterExample data = cser.deserializeNestedData<OuterExample, sergut::XmlValueType::Child>("outerTag", "innerTag");
   \endverbatim
 *
 * \subsection deserializeXmlDeserializerTiny2 XmlDeserializerTiny2
 * This deserializer is used as follows:
   \verbatim
    1 sergut::XmlDeserializerTiny2 ser(xml);
    2 const OuterExample data = cser.deserializeData<OuterExample>("outerTag");
   \endverbatim
 * AFAIK the only supported encoding for tinyxml is \c UTF-8.
 *
 * Note that (again) the outer tag of the XML has to be given to the function.
 *
 * Nested content is deserialized as follows:
   \verbatim
    1 sergut::XmlDeserializerTiny2 ser(xml);
    2 const OuterExample data = cser.deserializeNestedData<OuterExample, sergut::XmlValueType::Child>("outerTag", "innerTag");
   \endverbatim
 *
 */

/**
 * \brief Serialize a mandatory member.
 *
 * In case a mandatory member does not appear in an XML that is being
 * deserialized, a \c sergut::ParsingException is thrown unless it is a
 * collection.
 * \arg cls the name of the class instance thats member should be serialized.
 * \arg mem the name of the member of \c cls that should be serialized. For XML
 *      \c mem will also be the tag name.
 */
#define SERGUT_MMEMBER(cls, mem) \
  Archive::toNamedMember(#mem, cls.mem, true)

#define SERGUT_RENAMED_MMEMBER(memberAccess, serializedName) \
  Archive::toNamedMember(serializedName, memberAccess, true)

/**
 * \brief Serialize an optional member.
 *
 * In case an optional member does not appear in an XML that is being
 * deserialized, the member is left unchanged. Usually this means it is in its
 * default constructed state.
 * \arg cls the name of the class instance thats member should be serialized.
 * \arg mem the name of the member of \c cls that should be serialized. For XML
 *      \c mem will also be the tag name.
 */
#define SERGUT_OMEMBER(cls, mem) \
  Archive::toNamedMember(#mem, cls.mem, false)

#define SERGUT_RENAMED_OMEMBER(memberAccess, serializedName) \
  Archive::toNamedMember(serializedName, memberAccess, false)

/**
 * \brief Serialize a nested mandatory member.
 *
 * This is serialized as a nested XML as in
   \verbatim
   <mem><innerName>Data</innerName></mem>
   \endverbatim
 *
 * In case a mandatory member does not appear in an XML that is being
 * deserialized, a \c sergut::ParsingException is thrown unless it is a
 * collection.
 * \arg cls the name of the class instance thats member should be serialized.
 * \arg mem the name of the member of \c cls that should be serialized. For XML
 *      \c mem will also be the outer tag name.
 * \arg innerName the inner tag name of XML.
 */
#define SERGUT_NESTED_MMEMBER(cls, mem, innerName) \
  Archive::toNamedMember(#mem, Archive::toNestedMember(#innerName, cls.mem, true), true)

/**
 * \brief Serialize a nested optional member.
 *
 * This is serialized as a nested XML as in
   \verbatim
   <mem><innerName>Data</innerName></mem>
   \endverbatim
 *
 * In case an optional member does not appear in an XML that is being
 * deserialized, the member is left unchanged. Usually this means it is in its
 * default constructed state.
 * \arg cls the name of the class instance thats member should be serialized.
 * \arg mem the name of the member of \c cls that should be serialized. For XML
 *      \c mem will also be the outer tag name.
 * \arg innerName the inner tag name of XML.
 */
#define SERGUT_NESTED_OMEMBER(cls, mem, innerName) \
  Archive::toNamedMember(#mem, Archive::toNestedMember(#innerName, cls.mem, false), false)

/**
 * \brief Declaration of the deserialization/serialization function.
 */
#define SERGUT_FUNCTION(DataType, dataName, archiveName) \
  inline const char* getTypeName(const DataType*) { return #DataType; } \
  template<typename DT, typename Archive> \
  void serialize(Archive& archiveName, DT& dataName, const DataType*)

/**
 * \brief Friend declaration of the deserialization/serialization function.
 */
#define SERGUT_FUNCTION_FRIEND_DECL(DataType, dataName, archiveName) \
  template<typename DT, typename Archive> \
  friend \
  void serialize(Archive& archiveName, DT& dataName, const DataType*)

/**
 * \brief Declaration of the serialize to string function.
 *
 * This can be used for classes that should be serialized into a plain string.
 */
#define SERGUT_SERIALIZE_TO_STRING(DT, variableName) \
  inline const char* getTypeName(const DT*) { return #DT; } \
  inline std::string serializeToString(const DT& variableName)

/**
 * \brief Declaration of the serialize to string function (when moving impl to file).
 *
 * \see SERGUT_SERIALIZE_TO_STRING
 */
#define SERGUT_SERIALIZE_TO_STRING_DECL(DT, variableName) \
  inline const char* getTypeName(const DT*) { return #DT; } \
  std::string serializeToString(const DT& variableName)

/**
 * \brief Definition of the serialize to string function (when moving impl to file).
 *
 * \see SERGUT_SERIALIZE_TO_STRING
 */
#define SERGUT_SERIALIZE_TO_STRING_DEF(DT, variableName) \
  std::string serializeToString(const DT& variableName)

/**
 * \brief Friend declaration of the serialize to string function.
 */
#define SERGUT_SERIALIZE_TO_STRING_FRIEND_DECL(DT, variableName) \
  friend std::string serializeToString(const DT& variableName)

/**
 * \brief Declaration of the deserialize from string function.
 *
 * This can be used for classes that should be deserialized from a plain string.
 */
#define SERGUT_DESERIALIZE_FROM_STRING(DT, variableName, stringVariableName) \
  inline void deserializeFromString(DT& variableName, const std::string& stringVariableName)

/**
 * \brief Declaration of the deserialize from string function (when moving impl to file).
 *
 * \see SERGUT_DESERIALIZE_FROM_STRING
 */
#define SERGUT_DESERIALIZE_FROM_STRING_DECL(DT, variableName, stringVariableName) \
  void deserializeFromString(DT& variableName, const std::string& stringVariableName)

/**
 * \brief Definition of the deserialize from string function (when moving impl to file).
 *
 * \see SERGUT_DESERIALIZE_FROM_STRING
 */
#define SERGUT_DESERIALIZE_FROM_STRING_DEF(DT, variableName, stringVariableName) \
  void deserializeFromString(DT& variableName, const std::string& stringVariableName)


/**
 * \brief Friend declaration of the deserialize from string function.
 */
#define SERGUT_DESERIALIZE_FROM_STRING_FRIEND_DECL(DT, variableName, stringVariableName) \
  friend void deserializeFromString(DT& variableName, const std::string& stringVariableName)

namespace sergut {

struct ChildrenFollow {};
/**
 * \brief Marker variable that is used to signal that all member should be
 * serialized as XML children.
 */
const ChildrenFollow children;

struct PlainChildFollows {};
/**
 * \brief Marker variable that is used to signal that the next member should be
 * serialized as plain child.
 * \see plainChild
 */
const PlainChildFollows plainChild;

}
