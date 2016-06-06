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

#include "unicode/Utf32Char.h"
#include "xml/PullParser.h"
#include "xml/detail/ReaderState.h"
#include "xml/detail/ReaderStateResetter.h"
#include "xml/detail/TextDecodingHelper.h"

#include <string>
#include <vector>
#include <memory>

namespace xml {
namespace detail {

/**
 * @brief The PullParserNV class implements a very simple XML-Pull-Parser
 *
 * It does no validating. It does not even check that closing tags match their
 * opening tags. In case you want to verify this, the using application has
 * to do so.
 *
 * The parser operates on a constant buffer that it receives from outside.
 */
template<typename CharDecoder>
class BasicPullParserNV: public PullParser
{
public:
  typedef CharDecoder CharDecoderT;
  enum class NameType {
    Tag,
    Attribute,
  };

  BasicPullParserNV(const misc::StringRef& data);
  BasicPullParserNV(std::vector<char>&& data);
  std::vector<char>&& extractXmlData() override;
  ParseTokenType parseNext() override;
  ParseTokenType getCurrentTokenType() const override;
  misc::StringRef getCurrentTagName() const override;
  misc::StringRef getCurrentAttributeName() const override;
  DecodingResult getCurrentValue(char* buf, const std::size_t bufSize) const override;
  using PullParser::getCurrentValue;

private:
  /** Check whether there is an XML declaration and if so whether it is correct
   * @return true if there is no XML declaration or there is one that is OK (right version and encoding)
   */
  bool handleXmlDecl();
  bool skipWhitespaces();
  /**
   * Jump over the current character and set incompleteDocument == true in case it reaches the end of the buffer.
   * \return \c true if the following char is readable
   */
  bool nextChar();
  bool nextAsciiChar();
  /** ensures decodedName is sufficiently large to read the next UTF-8 char.
   * In case memory allocation fails, the currentTokenType is set to
   * ParseTokenType::Error.
   * \return false in case memory allocation failed.
   */
  bool ensureDecodedNameCapacity(char** currentWritePointer);
  /** writes a char to the buffer pointed to by \c currentWritePointer.
   * In case of an error the currentTokenType is set to ParseTokenType::Error.
   * /return false in case the character could not be written or memory allocation failed.
   */
  bool writeNameChar(char** currentWritePointer);

  // The parseXXX() functions return \c true, if their type of text has been handled.
  // In case of an error, they return \c true and set the error.
  bool parseName(const NameType nameType);
  bool parseAfterTag();
  bool parseOpenTag();
  bool parseAttribute(const bool setCurrentTokenTypeToAttribute);
  bool parseText();
  bool parseCloseTag();
  bool atEnd() const;
  unicode::Utf32Char peekChar() const;

private:
  friend class xml::detail::ReaderStateResetter;
  std::vector<char> inputData;
  std::vector<char> decodedName;
  ReaderState readerState;
  std::size_t descendCount = 0;
  ParseTokenType currentTokenType = ParseTokenType::InitialState;
  bool incompleteDocument = true;
};
}
}

template<typename CharDecoder>
xml::detail::BasicPullParserNV<CharDecoder>::BasicPullParserNV(const misc::StringRef& data)
  : BasicPullParserNV(std::vector<char>(data.begin(), data.end()))
{ }

template<typename CharDecoder>
xml::detail::BasicPullParserNV<CharDecoder>::BasicPullParserNV(std::vector<char>&& data)
  : inputData(std::move(data))
  , readerState(&*inputData.begin())
{
  readerState.readPointer = &*inputData.begin();
  if(incompleteDocument) {
    incompleteDocument = false;
    nextChar();
  }
}

template<typename CharDecoder>
std::vector<char>&& xml::detail::BasicPullParserNV<CharDecoder>::extractXmlData()
{
  incompleteDocument = true;
  return std::move(inputData);
}

template<typename CharDecoder>
misc::StringRef xml::detail::BasicPullParserNV<CharDecoder>::getCurrentTagName() const
{
  return misc::StringRef(readerState.startOfCurrentTagNamePointer, readerState.endOfCurrentTagNamePointer);
}

template<typename CharDecoder>
misc::StringRef xml::detail::BasicPullParserNV<CharDecoder>::getCurrentAttributeName() const
{
  return misc::StringRef(readerState.startOfCurrentAttrNamePointer, readerState.endOfCurrentAttrNamePointer);
}

template<typename CharDecoder>
xml::PullParser::DecodingResult xml::detail::BasicPullParserNV<CharDecoder>::getCurrentValue(char* buf, const std::size_t bufSize) const
{
  TextDecodingHelper<CharDecoder> helper(buf, buf + bufSize, TextDecodingHelper<CharDecoder>::TextType::Plain,
                            readerState.startOfCurrentValuePointer, readerState.endOfCurrentValuePointer);
  if(!helper.decodeText()) {
    return DecodingResult::Error;
  }
  return DecodingResult(helper.getWriteCount());
}

template<typename CharDecoder>
inline
bool xml::detail::BasicPullParserNV<CharDecoder>::skipWhitespaces()
{
  while(std::isspace(readerState.currentChar) && nextChar()) { }
  return !incompleteDocument && currentTokenType != ParseTokenType::Error;
}

template<typename CharDecoder>
inline
bool xml::detail::BasicPullParserNV<CharDecoder>::nextChar()
{
  const unicode::ParseResult parseResult =
      CharDecoderT::parseNext(readerState.currentChar, readerState.readPointer, &*inputData.end());
  if(!unicode::isError(parseResult)) {
    readerState.readPointer += static_cast<int32_t>(parseResult);
    return true;
  }
  switch (parseResult) {
  case unicode::ParseResult::IncompleteCharacter:
    incompleteDocument = true;
    return false;
  case unicode::ParseResult::InvalidCharacter:
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  assert(false);
}

template<typename CharDecoder>
inline
bool xml::detail::BasicPullParserNV<CharDecoder>::nextAsciiChar()
{
  if(!nextChar()) {
    return false;
  }
  if(readerState.currentChar > 0x7F) {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  return true;
}


template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::ensureDecodedNameCapacity(char** currentWritePointer)
{
  if(currentWritePointer == nullptr || *currentWritePointer + 4 > &*decodedName.end()) {
    try {
      const std::size_t startOfCurrentTagNameOffset  = readerState.startOfCurrentTagNamePointer  - decodedName.data();
      const std::size_t endOfCurrentTagNameOffset    = readerState.endOfCurrentTagNamePointer    - decodedName.data();
      const std::size_t startOfCurrentAttrNameOffset = readerState.startOfCurrentAttrNamePointer - decodedName.data();
      const std::size_t endOfCurrentAttrNameOffset   = readerState.endOfCurrentAttrNamePointer   - decodedName.data();
      decodedName.resize(decodedName.size() + 50);
      readerState.startOfCurrentTagNamePointer  = decodedName.data() + startOfCurrentTagNameOffset;
      readerState.endOfCurrentTagNamePointer    = decodedName.data() + endOfCurrentTagNameOffset;
      readerState.startOfCurrentAttrNamePointer = decodedName.data() + startOfCurrentAttrNameOffset;
      readerState.endOfCurrentAttrNamePointer   = decodedName.data() + endOfCurrentAttrNameOffset;
    } catch(const std::exception&) {
      currentTokenType = ParseTokenType::Error;
      return false;
    }
  }
  return true;
}

template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::writeNameChar(char** currentWritePointer)
{
  if(!ensureDecodedNameCapacity(currentWritePointer)) { return false; }
  const unicode::ParseResult res = unicode::Utf8Codec::encodeChar(readerState.currentChar, *currentWritePointer, &*decodedName.end());
  if(unicode::isError(res)) {
    currentTokenType = ParseTokenType::Error;
    return true;
  }
  *currentWritePointer += static_cast<std::size_t>(res);
  return true;
}

template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::handleXmlDecl()
{
//  [22]   	prolog	   ::=   	XMLDecl? Misc* (doctypedecl Misc*)?
//  [23]   	XMLDecl	   ::=   	'<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
//  [24]   	VersionInfo	   ::=   	S 'version' Eq ("'" VersionNum "'" | '"' VersionNum '"')
//  [25]   	Eq	   ::=   	S? '=' S?
//  [26]   	VersionNum	   ::=   	'1.' [0-9]+
//  [27]   	Misc	   ::=   	Comment | PI | S

  if(readerState.currentChar != '<') return true;
  ReaderStateResetter readStateResetter(readerState);
  if(!nextChar()) { return false; }
  if(readerState.currentChar != '?') return true;
  if(!nextChar()) { return false; }
  if(!parseName(NameType::Tag)) {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  if(getCurrentTagName() != misc::StringRef("xml")) {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  if(!skipWhitespaces()) { return false; }

  while(parseAttribute(false)) {
    misc::StringRef attrName(readerState.startOfCurrentAttrNamePointer, readerState.endOfCurrentAttrNamePointer);
    if(attrName == misc::StringRef("version")) {
      const misc::StringRef ver = getCurrentValue();
      if(ver.size() < 3 || ver[0] != '1' || ver[1] != '.') {
        // Wrong version
        currentTokenType = ParseTokenType::Error;
        return false;
      }
    } else if(attrName == misc::StringRef("encoding")) {
      if(!CharDecoder::isSupportedEncoding(misc::StringRef(getCurrentValue()))) {
        currentTokenType= ParseTokenType::Error;
        return false;
      }
    }
  }

  if(readerState.currentChar != '?') {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  if(!nextChar()) {
    return false;
  }
  if(readerState.currentChar != '>') {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  if(!nextChar()) {
    return false;
  }
  readStateResetter.release();
  return true;
}

template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::parseName(const NameType nameType)
{
  // [4] NameStartChar ::=  ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] |
  //                        [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] |
  //                        [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
  // [4a] NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
  if(!Helper::isNameStartChar(readerState.currentChar)) { return false; }
  if(!ensureDecodedNameCapacity(nullptr)) { return true; }
  switch (nameType) {
  case NameType::Tag:
    readerState.startOfCurrentTagNamePointer = &*decodedName.begin();
    readerState.endOfCurrentTagNamePointer   = readerState.startOfCurrentTagNamePointer;
    // fall through
  case NameType::Attribute:
    readerState.startOfCurrentAttrNamePointer = readerState.endOfCurrentTagNamePointer;
    readerState.endOfCurrentAttrNamePointer   = readerState.startOfCurrentAttrNamePointer;
  }
  char* currentWritePointer = const_cast<char*>(nameType==NameType::Tag
                                                ? readerState.startOfCurrentTagNamePointer
                                                : readerState.startOfCurrentAttrNamePointer);
  if(!writeNameChar(&currentWritePointer)) { return true; }

  if(!nextChar()) return true;
  while(Helper::isNameChar(readerState.currentChar)) {
    if(!writeNameChar(&currentWritePointer)) { return true; }
    if(!nextChar()) { return true; }
  }
  switch(nameType) {
  case NameType::Tag:
    readerState.endOfCurrentTagNamePointer   = currentWritePointer;
    return true;
  case NameType::Attribute:
    readerState.endOfCurrentAttrNamePointer   = currentWritePointer;
    return true;
  }
  assert(false);
  return true;
}

template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::parseAfterTag()
{
  if(parseCloseTag()) {
    return true;
  }
  if(parseOpenTag()) {
    return true;
  }
  if(parseText()) {
    return true;
  }
//  if(parseComment()) {
//    return true;
//  }
//  if(parseProcessingInstruction()) {
//    // XML-Spec: 2.6 "Processing Instructions" ->  <? ... ?>
//    // [16] PI ::= '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
//    // [17] PITarget ::= Name - (('X' | 'x') ('M' | 'm') ('L' | 'l'))

//    // Must be passed through to the application

//    return true;
//  }
  return false;
}

template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::parseOpenTag()
{
// [40] STag ::= '<' Name (S Attribute)* S? '>'
  if(readerState.currentChar != '<') { return false; }
  ReaderStateResetter readStateResetter(readerState);
  if(!nextChar())               { return true;  }
  if(!parseName(NameType::Tag)) { return false; }
  if(!skipWhitespaces())        { return true;  }
  readStateResetter.release();
  ++descendCount;
  currentTokenType = ParseTokenType::OpenTag;
  return true;
}

template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::parseAttribute(const bool setCurrentTokenTypeToAttribute)
{
  // [10] AttValue ::= '"' ([^<&"] | Reference)* '"' |
  // [25] Eq        ::= S? '=' S?
  // [41] Attribute ::= Name Eq AttValue
  //                   "'" ([^<&'] | Reference)* "'"
  // [67] Reference ::= EntityRef | CharRef
  // [68] EntityRef ::= '&' Name ';'

  // Characters must be in range
  //  [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]

  if(!parseName(NameType::Attribute)) {
    // No name, so we don't have an Attribute
    return false;
  }
  if(!skipWhitespaces()) { return true; }
  if(readerState.currentChar != '=') {
    currentTokenType = ParseTokenType::Error;
    return true;
  }
  if(!nextChar()) { return true; }
  if(!skipWhitespaces()) { return true; }
  if(readerState.currentChar != '"' && readerState.currentChar != '\'') {
    currentTokenType = ParseTokenType::Error;
    return true;
  }
  const typename TextDecodingHelper<CharDecoder>::TextType tt =
      readerState.currentChar == '"' ? TextDecodingHelper<CharDecoder>::TextType::AttValueQuote
                                     : TextDecodingHelper<CharDecoder>::TextType::AttValueApos;

  TextDecodingHelper<CharDecoder> helper(nullptr, nullptr, tt, readerState.readPointer, &*inputData.end());
  if(!helper.decodeText()) {
    if(helper.isError()) {
      currentTokenType = ParseTokenType::Error;
    }
    if(helper.isIncomplete()) {
      incompleteDocument = true;
    }
    return true;
  }
  readerState.readPointer = helper.getReadPointer();
  if(!nextChar()) { return true; }
  if(!skipWhitespaces()) { return true; }
  readerState.startOfCurrentValuePointer = helper.getStartOfTextPointer();
  readerState.endOfCurrentValuePointer   = helper.getEndOfTextPointer();
  readerState.currentValueDecodedSize    = helper.getWriteCount();
  if(setCurrentTokenTypeToAttribute) {
    currentTokenType = ParseTokenType::Attribute;
  }
  return true;
}

template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::parseText()
{
  // [43] content ::= CharData? ((element | Reference | CDSect | PI | Comment) CharData?)*

  if(readerState.currentChar == '<') {
    return false;
  }

  TextDecodingHelper<CharDecoder> helper(nullptr, nullptr, TextDecodingHelper<CharDecoder>::TextType::CharData,
                                         readerState.readPointer - static_cast<std::size_t>(CharDecoder::encodeChar(readerState.currentChar)),
                                         &*inputData.end());
  if(!helper.decodeText()) {
    if(helper.isIncomplete()) {
      incompleteDocument = true;
      return true;
    }
    currentTokenType = ParseTokenType::Error;
    return true;
  }
  readerState.readPointer = helper.getEndOfTextPointer();
  if(!nextChar()) {
    return true;
  }

  readerState.startOfCurrentValuePointer = helper.getStartOfTextPointer();
  readerState.endOfCurrentValuePointer = helper.getEndOfTextPointer();
  readerState.currentValueDecodedSize = helper.getWriteCount();
  currentTokenType = ParseTokenType::Text;
  return true;
}

template<typename CharDecoder>
bool xml::detail::BasicPullParserNV<CharDecoder>::parseCloseTag()
{
  // [42] ETag ::= '</' Name S? '>'
  if(readerState.currentChar != '<') return false;
  ReaderStateResetter readStateResetter(readerState);
  if(!nextChar()) {
    return true;
  }
  if(readerState.currentChar != '/') return false;
  if(!nextChar()) {
    return true;
  }
  if(!parseName(NameType::Tag)) {
    currentTokenType = ParseTokenType::Error;
    return true;
  }
  if(!skipWhitespaces()) {
    return true;
  }
  if(readerState.currentChar != '>') {
    currentTokenType = ParseTokenType::Error;
    return true;
  }
  if(descendCount > 1) {
    // don't try to read past the last tag
    if(!nextChar()) {
      return true;
    }
  }
  readStateResetter.release();
  currentTokenType = ParseTokenType::CloseTag;
  return true;
}

//bool xml::PullParserUtf8::parseCDText()
//{
//  //  [18]   	CDSect	   ::=   	CDStart CData CDEnd
//  //  [19]   	CDStart	   ::=   	'<![CDATA['
//  //  [20]   	CData	   ::=   	(Char* - (Char* ']]>' Char*))
//  //  [21]   	CDEnd	   ::=   	']]>'
//}

template<typename CharDecoder>
xml::ParseTokenType xml::detail::BasicPullParserNV<CharDecoder>::parseNext()
{
  if(incompleteDocument) {
    return ParseTokenType::IncompleteDocument;
  }
  switch(currentTokenType) {
  case ParseTokenType::InitialState:
    //    skipBom();
    //    <?xml version="1.0" encoding="UTF-8" ?>
    //    <!DOCTYPE greeting [
    //      <!ELEMENT greeting (#PCDATA)>
    //    ]>
    //    <greeting>Hello, world!</greeting>
    //    skipXmlDeclaration();
    //    skipXmlProcessingInstructions();
    //    skipXmlDtd();
    if(!handleXmlDecl()) {
      return getCurrentTokenType();
    }
    skipWhitespaces();
    if(currentTokenType == ParseTokenType::Error) {
      return currentTokenType;
    }
    if(incompleteDocument) {
      return ParseTokenType::IncompleteDocument;
    }
    currentTokenType = ParseTokenType::OpenDocument;
    return getCurrentTokenType();
  case ParseTokenType::CloseDocument:
  case ParseTokenType::Error:
    // No further processing in these cases
    return currentTokenType;
  case ParseTokenType::OpenDocument:
    if(parseOpenTag()) {
      return getCurrentTokenType();
    }
    currentTokenType = ParseTokenType::Error;
    return getCurrentTokenType();
  case ParseTokenType::OpenTag:
  case ParseTokenType::Attribute:
    if(parseAttribute(true)) {
      return getCurrentTokenType();
    }
    if(readerState.currentChar == '>') {
      if(!nextChar()) {
        return getCurrentTokenType();
      }
      if(parseAfterTag()) {
        return getCurrentTokenType();
      }
    } else if(readerState.currentChar == '/') {
      if(!nextChar()) {
        return getCurrentTokenType();
      }
      if(readerState.currentChar != '>') {
        currentTokenType = ParseTokenType::Error;
        return currentTokenType;
      }
      currentTokenType = ParseTokenType::CloseTag;
      if(descendCount == 1) {
        // If we are on the last closing tag, we don't continue checking
        // otherwise we might get incomplete document errors.
        readerState.currentChar = '\0';
      } else {
        nextChar(); // don't check the error, as we return in any case
      }
      return getCurrentTokenType();
    }
    currentTokenType = ParseTokenType::Error;
    return getCurrentTokenType();
  case ParseTokenType::CloseTag:
    --descendCount;
    if(descendCount == 0) {
      currentTokenType = ParseTokenType::CloseDocument;
      return currentTokenType;
    }
    if(parseAfterTag()) {
      return getCurrentTokenType();
    }
    currentTokenType = ParseTokenType::Error;
    return getCurrentTokenType();
  case ParseTokenType::Text:
    if(parseCloseTag()) {
      return getCurrentTokenType();
    }
    if(parseOpenTag()) {
      return getCurrentTokenType();
    }
    currentTokenType = ParseTokenType::Error;
    return getCurrentTokenType();
  case ParseTokenType::IncompleteDocument:
    assert(false);
  }
  assert(false);
  return ParseTokenType::Error;
}

template<typename CharDecoder>
xml::ParseTokenType xml::detail::BasicPullParserNV<CharDecoder>::getCurrentTokenType() const
{
  if(currentTokenType == ParseTokenType::Error || currentTokenType == ParseTokenType::CloseDocument) {
    return currentTokenType;
  }
  if(incompleteDocument) {
    return ParseTokenType::IncompleteDocument;
  }
  return currentTokenType;
}
