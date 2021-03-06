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

#include "XmlDeserializer.h"

#include "sergut/ParsingException.h"
#include "sergut/xml/PullParser.h"
#include "sergut/misc/ReadHelper.h"

#include <map>
#include <sstream>

namespace sergut {

std::size_t XmlDeserializer::ErrorContext::getRow() const
{
//  if(node != nullptr) {
//    return node->Row();
//  }
  return std::size_t(-1);
}

std::size_t XmlDeserializer::ErrorContext::getColumn() const
{
//  if(node != nullptr) {
//    return node->Column();
//  }
  return std::size_t(-1);
}


template<typename DT>
void handleSimpleType(const NamedMemberForDeserialization<DT>& data, const XmlValueType valueType, xml::PullParser& currentNode)
{
  switch(valueType) {
  case XmlValueType::Attribute: {
    if(currentNode.getCurrentTokenType() != xml::ParseTokenType::Attribute) {
      throw ParsingException("Expecting Attribute but got something else", XmlDeserializer::ErrorContext(currentNode));
    }
    assert(currentNode.getCurrentAttributeName() == data.name);
    sergut::misc::ReadHelper::readInto(currentNode.getCurrentValue(), data.data);
    currentNode.parseNext();
    return;
  }
  case XmlValueType::Child: {
    if(currentNode.parseNext() != xml::ParseTokenType::Text) {
      if(data.mandatory) {
        throw ParsingException("Text missing for mandatory simple datatype", XmlDeserializer::ErrorContext(currentNode));
      }
      if(currentNode.getCurrentTokenType() != xml::ParseTokenType::CloseTag) {
        throw ParsingException("Got wrong datatype, expecting simple type", XmlDeserializer::ErrorContext(currentNode));
      }
      currentNode.parseNext();
      return;
    }
    sergut::misc::ReadHelper::readInto(currentNode.getCurrentValue(), data.data);
    if(currentNode.parseNext() != xml::ParseTokenType::CloseTag) {
      throw ParsingException("Expecting closing tag", XmlDeserializer::ErrorContext(currentNode));
    }
    // move the parser one element after the current one
    currentNode.parseNext();
    return;
  }
  case XmlValueType::SingleChild: {
    assert(currentNode.getCurrentTokenType() == xml::ParseTokenType::Text);
    const sergut::misc::ConstStringRef content = currentNode.getCurrentValue();
    if(content.empty() && data.mandatory) {
      throw ParsingException("Text missing for mandatory simple datatype", XmlDeserializer::ErrorContext(currentNode));
    }
    sergut::misc::ReadHelper::readInto(content, data.data);
    if(currentNode.parseNext() != xml::ParseTokenType::CloseTag) {
      throw ParsingException("Expecting closing Tag but got something else", XmlDeserializer::ErrorContext(currentNode));
    }
    // staying on the closing tag, as the SingleChild does not have own tags
    break;
  }
  }
}

XmlDeserializer::XmlDeserializer(const misc::ConstStringRef& xml)
    : ownXmlDocument(xml::PullParser::createParser(xml))
    , xmlDocument(&*ownXmlDocument)
{ }

XmlDeserializer::XmlDeserializer(std::vector<char>&& xml)
  : ownXmlDocument(xml::PullParser::createParser(std::move(xml)))
  , xmlDocument(&*ownXmlDocument)
{ }

XmlDeserializer::XmlDeserializer(xml::PullParser& currentXmlNode)
    : xmlDocument(&currentXmlNode)
{ }

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<long long>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<long>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<int>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<short>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<unsigned long long>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<unsigned long>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<unsigned int>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<unsigned short>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<unsigned char>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<bool>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<double>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<float>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<std::string>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

void XmlDeserializer::handleChild(const NamedMemberForDeserialization<char>& data, const XmlValueType valueType, xml::PullParser& state) {
  handleSimpleType(data, valueType, state);
}

static void skipText(xml::PullParser& parser)
{
  if(parser.getCurrentTokenType() == xml::ParseTokenType::Text) {
    parser.parseNext();
  }
}

static void skipSubTree(xml::PullParser& parser)
{
  if(!parser.isOk()) {
    throw ParsingException("Errors while parsing", XmlDeserializer::ErrorContext(parser));
  }
  assert(parser.getCurrentTokenType() == xml::ParseTokenType::OpenTag || parser.getCurrentTokenType() == xml::ParseTokenType::Attribute);
  std::vector<std::string> parseStack;
  parseStack.push_back(parser.getCurrentTagName().toString());
  while(!parseStack.empty()) {
    switch(parser.parseNext()) {
    case xml::ParseTokenType::OpenTag:
      parseStack.push_back(parser.getCurrentTagName().toString());
      break;
    case xml::ParseTokenType::CloseTag:
      if(parseStack.back() != parser.getCurrentTagName()) {
        throw ParsingException("Wrong closing Tag", XmlDeserializer::ErrorContext(parser));
      }
      parseStack.pop_back();
      break;
    default:
      if(!parser.isOk()) {
        throw ParsingException("Error with XML-Document", XmlDeserializer::ErrorContext(parser));
      }
      break;
    }
  }
  // jump after the closing tag
  parser.parseNext();
}

void XmlDeserializer::feedMembers(MyMemberDeserializer &retriever, xml::PullParser& state)
{
  // try to get Attributes
  while(state.getCurrentTokenType() == xml::ParseTokenType::Attribute) {
    std::shared_ptr<MyMemberDeserializer::HolderBase> memberHolder = retriever.popMember(state.getCurrentAttributeName().toString());
    if(!memberHolder) {
      std::cerr << "Attribute handler for '" << state.getCurrentAttributeName() << "' does not exist" << std::endl;
      state.parseNext();
      continue;
    }
    memberHolder->execute(state);
  }
  // here token type is either Text, OpenTag, or CloseTag

  // try to handle single child
  if(state.getCurrentTokenType() == xml::ParseTokenType::Text) {
    // SingleChild can either be a simpleType or StringSerializable
    std::shared_ptr<MyMemberDeserializer::HolderBase> memberHolder = retriever.popMember(detail::MemberDeserializerBase::SINGLE_CHILD);
    if(memberHolder) {
      const std::string tagName = state.getCurrentTagName().toString();
      memberHolder->execute(state);
      if(state.getCurrentTokenType() != xml::ParseTokenType::CloseTag) {
        throw ParsingException("Not correctly closing a SingleChild", XmlDeserializer::ErrorContext(state));
      }
      if(state.getCurrentTagName() != tagName) {
        throw ParsingException("Expecting closing Tag but got something else", XmlDeserializer::ErrorContext(state));
      }
    } else {
      state.parseNext();
    }
  }

  if(state.getCurrentTokenType() != xml::ParseTokenType::CloseTag &&
         state.getCurrentTokenType() != xml::ParseTokenType::OpenTag) {
    throw ParsingException("Expecting opening or closing tag", XmlDeserializer::ErrorContext(state));
  }

  // if there was no single child get child members
  while(state.getCurrentTokenType() == xml::ParseTokenType::OpenTag) {
    std::shared_ptr<MyMemberDeserializer::HolderBase> memberHolder = retriever.popMember(state.getCurrentTagName().toString());
    if(memberHolder) {
      memberHolder->execute(state);
    } else {
      skipSubTree(state);
    }
    skipText(state);
    if(state.getCurrentTokenType() != xml::ParseTokenType::CloseTag &&
           state.getCurrentTokenType() != xml::ParseTokenType::OpenTag) {
      throw ParsingException("Expecting opening or closing tag", XmlDeserializer::ErrorContext(state));
    }
  }
  // here the token type has to be a CloseTag
  if(state.getCurrentTokenType() != xml::ParseTokenType::CloseTag) {
    throw ParsingException("Expecting closing tag", XmlDeserializer::ErrorContext(state));
  }
  state.parseNext();
  skipText(state);
  const xml::ParseTokenType currentTokenType = state.getCurrentTokenType();
  if(currentTokenType != xml::ParseTokenType::OpenTag
     && currentTokenType != xml::ParseTokenType::CloseTag
     && currentTokenType != xml::ParseTokenType::CloseDocument)
  {
    throw ParsingException("Expecting opening or closing tag after element", XmlDeserializer::ErrorContext(state));
  }

  // finally check whether mandatory members are missing
  for(const std::pair<const std::string, std::shared_ptr<MyMemberDeserializer::HolderBase>>& e: retriever.getMembers()) {
    if(e.second->isMandatory() && !e.second->isContainer()) {
      throw ParsingException("Mandatory child '" + e.first + "' is missing", XmlDeserializer::ErrorContext(state));
    }
  }
}

std::string XmlDeserializer::popString(const XmlValueType valueType, xml::PullParser& state)
{
  switch(valueType) {
  case XmlValueType::Attribute: {
    if(state.getCurrentTokenType() != xml::ParseTokenType::Attribute) {
      throw ParsingException("Expecting Attribute, but got something else", XmlDeserializer::ErrorContext(state));
    }
    const std::string attrVal = state.getCurrentValue().toString();
    state.parseNext();
    return attrVal;
  }
  case XmlValueType::Child: {
    assert(state.getCurrentTokenType() == xml::ParseTokenType::OpenTag);
    if(state.parseNext() != xml::ParseTokenType::Text) {
      if(state.getCurrentTokenType() != xml::ParseTokenType::CloseTag) {
        throw ParsingException("String serializable child is missing and contains an opening tag", XmlDeserializer::ErrorContext(state));
      }
      state.parseNext();
      return std::string();
    }
    const std::string txt = state.getCurrentValue().toString();
    if(state.parseNext() != xml::ParseTokenType::CloseTag) {
      throw ParsingException("Expecting closing tag in string serializable element", XmlDeserializer::ErrorContext(state));
    }
    state.parseNext();
    return txt;
  }
  case XmlValueType::SingleChild: {
    assert(state.getCurrentTokenType() == xml::ParseTokenType::Text);
    const std::string txt = state.getCurrentValue().toString();
    if(state.parseNext() != xml::ParseTokenType::CloseTag) {
      throw ParsingException("expecting closing tag after poping single child", XmlDeserializer::ErrorContext(state));
    }
    // stay on the closing tag, as this is the one of the parent (SingleChilds don't have own tags)
    return txt;
  }
  }
  return std::string();
}

bool XmlDeserializer::checkNextContainerElement(const char* name, const XmlValueType valueType, xml::PullParser& state)
{
  switch(valueType) {
  case XmlValueType::Attribute: {
    return state.getCurrentTokenType() == xml::ParseTokenType::Attribute && state.getCurrentAttributeName() == name;
  }
  case XmlValueType::Child: {
    return state.getCurrentTokenType() == xml::ParseTokenType::OpenTag && state.getCurrentTagName() == name;
  }
  case XmlValueType::SingleChild:
    break;
  }
  return false;
}

} // namespace sergut
