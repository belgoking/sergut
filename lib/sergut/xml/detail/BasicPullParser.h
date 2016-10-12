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

#include "sergut/unicode/Utf32Char.h"
#include "sergut/xml/PullParser.h"
#include "sergut/xml/detail/ParseStack.h"
#include "sergut/xml/detail/ReaderState.h"
#include "sergut/xml/detail/ReaderStateResetter.h"
#include "sergut/xml/detail/TextDecodingHelper.h"

#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace sergut {
namespace xml {
namespace detail {

template<bool isUtf8>
struct DecodedNameBuffers;

template<>
struct DecodedNameBuffers<true> {
public:
  void addOffset(const std::ptrdiff_t offset) {
    decodedTagName.addOffset(offset);
    decodedAttrName.addOffset(offset);
  }
public:
  sergut::misc::ConstStringRef decodedTagName;
  sergut::misc::ConstStringRef decodedAttrName;
};

template<>
struct DecodedNameBuffers<false> {
public:
  void addOffset(const std::ptrdiff_t offset) { (void)offset; }
public:
  std::vector<char>       decodedTagNameBuffer;
  sergut::misc::StringRef decodedTagName;
  std::vector<char>       decodedAttrNameBuffer;
  sergut::misc::StringRef decodedAttrName;
};

/**
 * @brief The BasicPullParser class implements a very simple XML-Pull-Parser
 *
 * The parser operates on a constant buffer that it receives from outside.
 */
template<typename CharDecoder>
class BasicPullParser: public PullParser
{
public:
  typedef CharDecoder CharDecoderT;
  enum class NameType {
    Tag,
    Attribute,
  };

  // The InnerStateSavePoint is allways on the beginning of a tag, be it an opening or
  // a closing tag. Thus when restoring, the tag has to be parsed, such that the
  // currentTokenType can be deduced.
  struct InnerStateSavePoint {
    InnerStateSavePoint() = default;
    InnerStateSavePoint(const InnerStateSavePoint&) = delete;
    InnerStateSavePoint(const InnerStateSavePoint&&) = delete;
    InnerStateSavePoint& operator=(const InnerStateSavePoint&) = delete;
    InnerStateSavePoint& operator=(const InnerStateSavePoint&&) = delete;

    void addOffset(const std::ptrdiff_t offset);
    bool hasParseStackCopy() const;
    void aboutToPopParseStack();

  public:
    const char* readPointer = nullptr;
    // the parseStack is merely copied if required. While the stack is larger than it was at
    // the save-point we just memorize its size
    ParseStack<std::is_same<CharDecoder, sergut::unicode::Utf8Codec>::value> parseStackCopy;
    ParseStack<std::is_same<CharDecoder, sergut::unicode::Utf8Codec>::value>* parseStackPtr = nullptr;
    std::size_t parseStackSize = 0;
  };

  BasicPullParser(const sergut::misc::ConstStringRef& data);
  BasicPullParser(std::vector<char>&& data, const std::size_t offset);
  std::vector<char>&& extractXmlData() override;
  ParseTokenType parseNext() override;
  ParseTokenType getCurrentTokenType() const override;
  sergut::misc::ConstStringRef getCurrentTagName() const override;
  sergut::misc::ConstStringRef getCurrentAttributeName() const override;
  sergut::misc::ConstStringRef getCurrentValue() const override;
  void appendData(const char* data, const std::size_t size) override;

  bool setSavePoint() override;
  bool restoreToSavePoint() override;

private:
  void compressInnerData();
  void recomputePointersToInput(const char* oldStartOfInput);
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
  bool ensureDecodedNameCapacity(std::vector<char>& nameBuffer, sergut::misc::StringRef& name);
  /** writes a char to the buffer pointed to by \c currentWritePointer.
   * In case of an error the currentTokenType is set to ParseTokenType::Error.
   * /return false in case the character could not be written or memory allocation failed.
   */
  bool writeNameChar(std::vector<char>& nameBuffer, sergut::misc::StringRef& name);

  // The parseXXX() functions return \c true, if their type of text has been handled.
  // In case of an error, they return \c true and set the error.
  bool parseName(const NameType nameType);
  bool parseAfterTag();
  bool parseOpenTag();
  bool parseAttribute(const bool setCurrentTokenTypeToAttribute);
  bool parseText();
  bool parseCloseTag();
  bool atEnd() const;
  sergut::unicode::Utf32Char peekChar() const;

private:
  friend class sergut::xml::detail::ReaderStateResetter;
  std::vector<char> inputData;
  ReaderState readerState;
  ParseStack<std::is_same<CharDecoder, sergut::unicode::Utf8Codec>::value> parseStack;

  DecodedNameBuffers<std::is_same<CharDecoder, sergut::unicode::Utf8Codec>::value> decodedNameBuffers;
  std::vector<char> decodedValueBuffer;

  ParseTokenType currentTokenType = ParseTokenType::InitialState;
  bool incompleteDocument = true;

  // Variables needed for safe/restore
  const char* lastTagStart = nullptr;
  std::unique_ptr<InnerStateSavePoint> innerStateSavePoint;
};

}
}
}

template<typename CharDecoder>
inline
void sergut::xml::detail::BasicPullParser<CharDecoder>::InnerStateSavePoint::addOffset(const std::ptrdiff_t offset)
{
  readPointer += offset;
}

template<typename CharDecoder>
inline
bool sergut::xml::detail::BasicPullParser<CharDecoder>::InnerStateSavePoint::hasParseStackCopy() const
{
  return parseStackPtr == &parseStackCopy;
}

template<typename CharDecoder>
inline
void sergut::xml::detail::BasicPullParser<CharDecoder>::InnerStateSavePoint::aboutToPopParseStack()
{
  // if we hold a copy of the parseStack or if the stack is larger than it was when we took
  // the savepoint there is nothing to do, as the part of the stack that represents the state
  // at the time of the savepoint is still there
  if(hasParseStackCopy() || parseStackPtr->frameCount() != parseStackSize) {
    return;
  }
  parseStackCopy = *parseStackPtr; // copy the data
  parseStackPtr = &parseStackCopy; // then point the pointer to the copy
}


template<typename CharDecoder>
sergut::xml::detail::BasicPullParser<CharDecoder>::BasicPullParser(const sergut::misc::ConstStringRef& data)
  : BasicPullParser(std::vector<char>(data.begin(), data.end()), 0)
{ }

template<typename CharDecoder>
sergut::xml::detail::BasicPullParser<CharDecoder>::BasicPullParser(std::vector<char>&& data, const std::size_t offset)
  : inputData(std::move(data))
  , readerState(&*inputData.begin() + offset)
{
  if(incompleteDocument) {
    incompleteDocument = false;
    nextChar();
  }
}

template<typename CharDecoder>
std::vector<char>&& sergut::xml::detail::BasicPullParser<CharDecoder>::extractXmlData()
{
  incompleteDocument = true;
  return std::move(inputData);
}

template<typename CharDecoder>
sergut::misc::ConstStringRef sergut::xml::detail::BasicPullParser<CharDecoder>::getCurrentTagName() const
{
  return parseStack.getTopData();
}

template<typename CharDecoder>
sergut::misc::ConstStringRef sergut::xml::detail::BasicPullParser<CharDecoder>::getCurrentAttributeName() const
{
  return decodedNameBuffers.decodedAttrName;
}

template<typename CharDecoder>
sergut::misc::ConstStringRef sergut::xml::detail::BasicPullParser<CharDecoder>::getCurrentValue() const
{
  return sergut::misc::ConstStringRef(&*decodedValueBuffer.begin(), &*decodedValueBuffer.end());
}

template<typename CharDecoder>
void sergut::xml::detail::BasicPullParser<CharDecoder>::appendData(const char* data, const std::size_t size)
{
  compressInnerData();
  const char* oldStartPos = inputData.data();
  inputData.insert(inputData.end(), data, data + size);
  recomputePointersToInput(oldStartPos);
}

template<typename CharDecoder>
bool sergut::xml::detail::BasicPullParser<CharDecoder>::setSavePoint()
{
  if(incompleteDocument
     || currentTokenType == ParseTokenType::Error
     || currentTokenType == ParseTokenType::InitialState)
  {
    return false;
  }
  if(!innerStateSavePoint) {
    innerStateSavePoint.reset(new InnerStateSavePoint);
  }
  // we could call innerStateSavePoint->parseStackCopy.clear(), but why should
  // we? The memory consumption of the saved stack should be neglectable and by
  // not calling clear we save the effort of some calls to delete
  innerStateSavePoint->readPointer = lastTagStart;
  innerStateSavePoint->parseStackPtr = &parseStack;
  innerStateSavePoint->parseStackSize = parseStack.frameCount();
  if(currentTokenType != ParseTokenType::CloseTag) {
    // As the savepoint is set before the last opening tag we have
    // to reduce the stack size by one, as returning to the savepoint
    // we still have to visit the last stack frame.
    innerStateSavePoint->parseStackSize -= 1;
  }
  return true;
}

template<typename CharDecoder>
bool sergut::xml::detail::BasicPullParser<CharDecoder>::restoreToSavePoint()
{
  if(!innerStateSavePoint) {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  lastTagStart = innerStateSavePoint->readPointer;
  readerState.readPointer = lastTagStart;
  incompleteDocument = false;

  if(innerStateSavePoint->hasParseStackCopy()) {
    parseStack = std::move(innerStateSavePoint->parseStackCopy);
    innerStateSavePoint->parseStackPtr = &parseStack;
    innerStateSavePoint->parseStackSize = parseStack.frameCount();
  } else {
    while(innerStateSavePoint->parseStackPtr->frameCount() > innerStateSavePoint->parseStackSize) {
      innerStateSavePoint->parseStackPtr->popData();
    }
  }
  if(!nextAsciiChar()) {
    return false;
  }
  if(parseCloseTag()) {
    return true;
  }
  if(parseOpenTag()) {
    return true;
  }
  currentTokenType = ParseTokenType::Error;
  return false;
}


template<typename CharDecoder>
void sergut::xml::detail::BasicPullParser<CharDecoder>::compressInnerData()
{
  // implement compressInnerData() for UTF-16

  // the save point is allways at least before the read pointer
  const char* storedReadPtrPos = innerStateSavePoint != nullptr ? innerStateSavePoint->readPointer : readerState.readPointer;

  const std::size_t reduceBy = storedReadPtrPos - inputData.data();
  const std::size_t remainingDataSize = (&*inputData.end()) - storedReadPtrPos;
  std::memcpy(inputData.data(), storedReadPtrPos, remainingDataSize);

  if(innerStateSavePoint != nullptr) {
    innerStateSavePoint->readPointer -= reduceBy;
  }
  readerState.readPointer -= reduceBy;
  inputData.erase(inputData.begin()+remainingDataSize, inputData.end());
}


template<typename CharDecoder>
void sergut::xml::detail::BasicPullParser<CharDecoder>::recomputePointersToInput(const char* oldStartOfInput)
{
  if(oldStartOfInput == inputData.data()) {
    return;
  }
  const std::ptrdiff_t diff = inputData.data() - oldStartOfInput;
  readerState.readPointer += diff;
  parseStack.addOffset(diff);
  decodedNameBuffers.addOffset(diff);
  if(innerStateSavePoint) {
    innerStateSavePoint->addOffset(diff);
  }
}

template<typename CharDecoder>
inline
bool sergut::xml::detail::BasicPullParser<CharDecoder>::skipWhitespaces()
{
  while(std::isspace(readerState.currentChar) && nextChar()) { }
  return isOk();
}

template<typename CharDecoder>
inline
bool sergut::xml::detail::BasicPullParser<CharDecoder>::nextChar()
{
  const sergut::unicode::ParseResult parseResult =
      CharDecoderT::parseNext(readerState.currentChar, readerState.readPointer, &*inputData.end());
  if(!sergut::unicode::isError(parseResult)) {
    readerState.readPointer += static_cast<int32_t>(parseResult);
    return true;
  }
  switch (parseResult) {
  case sergut::unicode::ParseResult::IncompleteCharacter:
    incompleteDocument = true;
    return false;
  case sergut::unicode::ParseResult::InvalidCharacter:
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  assert(false);
}

template<typename CharDecoder>
inline
bool sergut::xml::detail::BasicPullParser<CharDecoder>::nextAsciiChar()
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
bool sergut::xml::detail::BasicPullParser<CharDecoder>::ensureDecodedNameCapacity(std::vector<char>& nameBuffer, sergut::misc::StringRef& name)
{
  try {
    if(&*nameBuffer.end() - name.end() < 4) {
      const std::size_t offset = name.end() - nameBuffer.data();
      nameBuffer.resize(nameBuffer.size() + 50);
      name = sergut::misc::StringRef(nameBuffer.data(), nameBuffer.data() + offset);
    }
  } catch(const std::exception&) {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  return true;
}

template<typename CharDecoder>
bool sergut::xml::detail::BasicPullParser<CharDecoder>::writeNameChar(std::vector<char>& nameBuffer, sergut::misc::StringRef& name)
{
  if(!ensureDecodedNameCapacity(nameBuffer, name)) { return false; }
  const sergut::unicode::ParseResult res = sergut::unicode::Utf8Codec::encodeChar(readerState.currentChar, name.end(), &*nameBuffer.end());
  if(sergut::unicode::isError(res)) {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  name = sergut::misc::StringRef(name.begin(), name.end() + static_cast<std::size_t>(res));
  return true;
}

template<typename CharDecoder>
bool sergut::xml::detail::BasicPullParser<CharDecoder>::handleXmlDecl()
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
  if(!isOk()) { return false; }

  if(decodedNameBuffers.decodedTagName != sergut::misc::ConstStringRef("xml")) {
    currentTokenType = ParseTokenType::Error;
    return false;
  }
  if(!skipWhitespaces()) { return false; }

  while(parseAttribute(false)) {
    if(decodedNameBuffers.decodedAttrName == sergut::misc::ConstStringRef("version")) {
      const sergut::misc::ConstStringRef ver = getCurrentValue();
      if(ver.size() < 3 || ver[0] != '1' || ver[1] != '.') {
        // Wrong version
        currentTokenType = ParseTokenType::Error;
        return false;
      }
    } else if(decodedNameBuffers.decodedAttrName == sergut::misc::ConstStringRef("encoding")) {
      if(!CharDecoder::isSupportedEncoding(getCurrentValue())) {
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
bool sergut::xml::detail::BasicPullParser<CharDecoder>::parseName(const NameType nameType)
{
  // [4] NameStartChar ::=  ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] |
  //                        [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] |
  //                        [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
  // [4a] NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
  if(!Helper::isNameStartChar(readerState.currentChar)) { return false; }
  std::vector<char>& nameBuffer = (nameType == NameType::Tag) ? decodedNameBuffers.decodedTagNameBuffer
                                                              : decodedNameBuffers.decodedAttrNameBuffer;
  sergut::misc::StringRef&   name       = (nameType == NameType::Tag) ? decodedNameBuffers.decodedTagName
                                                              : decodedNameBuffers.decodedAttrName;

  // clean up
  name = sergut::misc::StringRef(name.begin(), name.begin());
  if(!writeNameChar(nameBuffer, name)) { return true; }

  if(!nextChar()) return true;
  while(Helper::isNameChar(readerState.currentChar)) {
    if(!writeNameChar(nameBuffer, name)) { return true; }
    if(!nextChar()) { return true; }
  }
  return true;
}

template<typename CharDecoder>
bool sergut::xml::detail::BasicPullParser<CharDecoder>::parseAfterTag()
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
bool sergut::xml::detail::BasicPullParser<CharDecoder>::parseOpenTag()
{
// [40] STag ::= '<' Name (S Attribute)* S? '>'
  if(readerState.currentChar != '<') { return false; }
  const char* tmpStartChar = readerState.readPointer;
  if(!nextChar())               { return true;  }
  if(!parseName(NameType::Tag)) { return false; }
  if(!isOk())                   { return true; }
  if(!skipWhitespaces())        { return true;  }
  // memorize the position of the opening '<' of the tag
  lastTagStart = tmpStartChar - std::size_t(CharDecoder::encodeChar('<'));
  parseStack.pushData(decodedNameBuffers.decodedTagName);
  currentTokenType = ParseTokenType::OpenTag;
  return true;
}

template<typename CharDecoder>
bool sergut::xml::detail::BasicPullParser<CharDecoder>::parseAttribute(const bool setCurrentTokenTypeToAttribute)
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
  if(!isOk())            { return true; }
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

  TextDecodingHelper<CharDecoder> helper(decodedValueBuffer, tt, readerState.readPointer, &*inputData.end());
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
  if(setCurrentTokenTypeToAttribute) {
    currentTokenType = ParseTokenType::Attribute;
  }
  return true;
}

template<typename CharDecoder>
bool sergut::xml::detail::BasicPullParser<CharDecoder>::parseText()
{
  // [43] content ::= CharData? ((element | Reference | CDSect | PI | Comment) CharData?)*

  if(readerState.currentChar == '<') {
    return false;
  }

  TextDecodingHelper<CharDecoder> helper(decodedValueBuffer, TextDecodingHelper<CharDecoder>::TextType::CharData,
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
  currentTokenType = ParseTokenType::Text;
  return true;
}

template<typename CharDecoder>
bool sergut::xml::detail::BasicPullParser<CharDecoder>::parseCloseTag()
{
  // [42] ETag ::= '</' Name S? '>'
  if(readerState.currentChar != '<') return false;
  ReaderStateResetter readStateResetter(readerState);
  if(!nextChar()) {
    return true;
  }
  if(readerState.currentChar != '/') return false;
  readStateResetter.release();
  if(!nextChar()) {
    return true;
  }
  if(!parseName(NameType::Tag)) {
    currentTokenType = ParseTokenType::Error;
    return true;
  }
  if(!isOk()) { return true; }
  if(decodedNameBuffers.decodedTagName != parseStack.getTopData()) {
    // tags don't match
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
  if(parseStack.frameCount() > 1) {
    // don't try to read past the last tag
    if(!nextChar()) {
      return true;
    }
  }
  // memorize the position of the opening '<' of the tag
  lastTagStart = readStateResetter.getOriginalReadPointer() - std::size_t(CharDecoder::encodeChar('<'));
  currentTokenType = ParseTokenType::CloseTag;
  return true;
}

//bool sergut::xml::PullParserUtf8::parseCDText()
//{
//  //  [18]   	CDSect	   ::=   	CDStart CData CDEnd
//  //  [19]   	CDStart	   ::=   	'<![CDATA['
//  //  [20]   	CData	   ::=   	(Char* - (Char* ']]>' Char*))
//  //  [21]   	CDEnd	   ::=   	']]>'
//}

template<typename CharDecoder>
sergut::xml::ParseTokenType sergut::xml::detail::BasicPullParser<CharDecoder>::parseNext()
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
      if(parseStack.frameCount() == 1) {
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
    if(innerStateSavePoint != nullptr) {
      innerStateSavePoint->aboutToPopParseStack();
    }
    parseStack.popData();
    if(parseStack.frameCount() == 0) {
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
sergut::xml::ParseTokenType sergut::xml::detail::BasicPullParser<CharDecoder>::getCurrentTokenType() const
{
  if(currentTokenType == ParseTokenType::Error || currentTokenType == ParseTokenType::CloseDocument) {
    return currentTokenType;
  }
  if(incompleteDocument) {
    return ParseTokenType::IncompleteDocument;
  }
  return currentTokenType;
}
