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

#include "sergut/unicode/Utf8Codec.h"
#include "sergut/xml/detail/BasicPullParser.h"
#include "sergut/xml/ParseTokenType.h"

namespace sergut {
namespace xml {
namespace detail {

class PullParserUtf8: public BasicPullParser<sergut::unicode::Utf8Codec>
{
  using BasicPullParser<sergut::unicode::Utf8Codec>::BasicPullParser;
};

template<>
inline
bool BasicPullParser<sergut::unicode::Utf8Codec>::nextAsciiChar()
{
  if(readerState.readPointer == &*inputData.end()) {
    incompleteDocument = true;
    return false;
  }
  if(!sergut::unicode::Utf8Codec::isAscii(*readerState.readPointer)) {
    currentTokenType = xml::ParseTokenType::Error;
    return false;
  }
  readerState.currentChar = *readerState.readPointer;
  ++readerState.readPointer;
  return true;
}

template<>
bool BasicPullParser<sergut::unicode::Utf8Codec>::parseName(const NameType nameType)
{
  // [4] NameStartChar ::=  ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] |
  //                        [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] |
  //                        [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
  // [4a] NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
  if(!Helper::isNameStartChar(readerState.currentChar)) return false;
  const char* startOfName = readerState.readPointer - std::size_t(sergut::unicode::Utf8Codec::encodeChar(readerState.currentChar));
  if(!nextChar()) return true;
  while(Helper::isNameChar(readerState.currentChar)) {
    if(!nextChar()) return true;
  }
  switch(nameType) {
  case NameType::Tag:
    // here it is OK to subtract 1 from readPointer, as we know the only possible chars are '>' or 0x9, 0xA, 0xD, 0x20
    decodedNameBuffers.decodedTagName = sergut::misc::ConstStringRef(startOfName, readerState.readPointer - 1);
    return true;
  case NameType::Attribute:
    // here it is OK to subtract 1 from readPointer, as we know the only possible chars are '=' or 0x9, 0xA, 0xD, 0x20
    decodedNameBuffers.decodedAttrName = sergut::misc::ConstStringRef(startOfName, readerState.readPointer - 1);
    return true;
  }
  assert(false);
  return true;
}

inline
std::size_t moveNReturnOffset(sergut::misc::ConstStringRef& ref, char* lastDataEnd) {
  sergut::misc::ConstStringRef orig = ref;
  std::memcpy(lastDataEnd, orig.begin(), orig.size());
  char* const newEnd = lastDataEnd + orig.size();
  const sergut::misc::ConstStringRef newPos(lastDataEnd, newEnd);
  ref = newPos;
  return orig.begin() - lastDataEnd;
}

template<>
void sergut::xml::detail::BasicPullParser<sergut::unicode::Utf8Codec>::recomputePointersToInput(const char* oldStartOfInput)
{
  if(oldStartOfInput == inputData.data()) {
    return;
  }
  const std::ptrdiff_t diff = inputData.data() - oldStartOfInput;
  lastTagStart += diff;
  readerState.readPointer += diff;
  parseStack.addOffset(diff);
  decodedNameBuffers.addOffset(diff);
  if(innerStateSavePoint) {
    innerStateSavePoint->addOffset(diff);
  }
}

template<>
void sergut::xml::detail::BasicPullParser<sergut::unicode::Utf8Codec>::compressInnerData()
{
  // implement compressInnerData() for UTF-8

  if(currentTokenType == ParseTokenType::Error) {
    // If we have an error, we don't try to do anything here
    return;
  }

  ParseStack<true>::iterator  mainStackIter = parseStack.begin();
  const ParseStack<true>::iterator  mainStackEndIter = parseStack.end();

  std::size_t offset = 0;
  char* writePointer = inputData.data();

  if(innerStateSavePoint) {
    // if we have a save point, we compact until innerStateSavePoint->readPointer
    ParseStack<true>::iterator savepointStackIter = innerStateSavePoint->parseStackPtr->begin();
    const ParseStack<true>::iterator savepointStackEndIter =
        innerStateSavePoint->parseStackPtr != &parseStack
          ? innerStateSavePoint->parseStackPtr->end()
          : (innerStateSavePoint->parseStackPtr->begin() + innerStateSavePoint->parseStackSize);

    for(;savepointStackIter != savepointStackEndIter; ++savepointStackIter) {
      sergut::misc::ConstStringRef orig = *savepointStackIter;
      offset = moveNReturnOffset(*savepointStackIter, writePointer);
      writePointer += savepointStackIter->size();
      if(&*mainStackIter == &*savepointStackIter) {
        // if innerStateSavePoint just points to the stack of the main
        // parser, we just need to increase the mainStackIter, as the
        // values of the ConstStringRef that are pointed to by it have
        // been increased already
        ++mainStackIter;
      } else if(orig.begin() == mainStackIter->begin()) {
        // if the current frame (previous to being moved) is the same for the
        // stored and the main ParseStack, we also move the frame on the
        // mainStackIter. We can do this, as we know that both stacks are
        // equal up to a certain point. After this point all pointers held by
        // the innerStateSavePoint are before all remaining pointers in the
        // main parseStack.
        *mainStackIter = *savepointStackIter;
        ++mainStackIter;
      }
    }
    innerStateSavePoint->readPointer -= offset;
    // then I have to fix the remaining positions of the main parseStack
    // (those it does not have in common with innerStateSavePoint->parseStackPtr
    for(;mainStackIter != mainStackEndIter; ++mainStackIter) {
      mainStackIter->addOffset(-offset);
    }
    // now I have to fix the other pointers
    decodedNameBuffers.addOffset(-offset);

    // copying of the data and trimming happens after the else block
  } else {
    // if we don't have a save point, we compact until the main readPointer

    if(incompleteDocument) {
      // we need a savepoint to recover from an incomplete document
      // hence we don't try to compact in this case
      return;
    }

    // first we compact the parse stack
    while(mainStackIter != mainStackEndIter) {
      offset = moveNReturnOffset(*mainStackIter, writePointer);
      writePointer += mainStackIter->size();
      ++mainStackIter;
    }

    // compact
    switch(currentTokenType) {
    case sergut::xml::ParseTokenType::OpenTag:
    case sergut::xml::ParseTokenType::CloseTag:
      decodedNameBuffers.decodedAttrName = sergut::misc::StringRef();
      decodedNameBuffers.decodedTagName.addOffset(-offset);
      break;
    case sergut::xml::ParseTokenType::Attribute:
      decodedNameBuffers.decodedTagName = sergut::misc::StringRef();
      decodedNameBuffers.decodedAttrName.addOffset(-offset);
      break;
    case sergut::xml::ParseTokenType::Text:
    case sergut::xml::ParseTokenType::InitialState:
    case sergut::xml::ParseTokenType::OpenDocument:
    case sergut::xml::ParseTokenType::CloseDocument:
      decodedNameBuffers.decodedTagName = sergut::misc::StringRef();
      decodedNameBuffers.decodedAttrName = sergut::misc::StringRef();
      break;
    case sergut::xml::ParseTokenType::IncompleteDocument:
      // This should never have happened. In case we want to work with
      // partial data we need a savepoint to recover.
    case sergut::xml::ParseTokenType::Error:
      return;
    }
  }
  const std::size_t remaining = &*inputData.end() - (writePointer + offset);
  std::memcpy(writePointer, writePointer + offset, remaining);
  lastTagStart -= offset;
  readerState.readPointer -= offset;

  std::size_t compressedDataSize = writePointer + remaining - inputData.data();

  const char* oldDataPtr = inputData.data();
  inputData.erase(inputData.begin()+compressedDataSize, inputData.end());
  recomputePointersToInput(oldDataPtr);
}

}
}
}
