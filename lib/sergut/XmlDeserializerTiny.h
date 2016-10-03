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

#include "sergut/detail/XmlDeserializerDomBase.h"

#include <tinyxml.h>

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

namespace detail {

struct TinyDom {

  typedef TiXmlDocument Document;
  typedef TiXmlNode Node;

  template<typename DT>
  static void extractAndDeleteAttribute(const NamedMemberForDeserialization<DT>& data, TiXmlNode& currentElement);

  template<typename DT>
  static void extractAndDeleteSimpleChild(const NamedMemberForDeserialization<DT>& data, TiXmlNode& currentElement);

  template<typename DT>
  static void extractAndDeleteSingleChild(const NamedMemberForDeserialization<DT>& data, TiXmlNode& currentElement);

  static TiXmlNode* getParentNode(TiXmlNode& currentElement);
  static void deleteChildNode(TiXmlNode& parentNode, TiXmlNode& childNode);
};

template<>
class XmlDeserializerDomBase<detail::TinyDom>::ErrorContext: public ParsingException::ErrorContext {
public:
  ErrorContext(const TiXmlNode& pNode) : node(&pNode) { }
  std::size_t getRow() const override {
    if(node != nullptr) {
      return node->Row();
    }
    return std::size_t(-1);
  }
  std::size_t getColumn() const override {
    if(node != nullptr) {
      return node->Column();
    }
    return std::size_t(-1);
  }

private:
  const TiXmlNode* node = nullptr;
};

template<>
XmlDeserializerDomBase<detail::TinyDom>::XmlDeserializerDomBase(const std::string& xml);

template<typename DT>
inline
void TinyDom::extractAndDeleteAttribute(const NamedMemberForDeserialization<DT>& data, TiXmlNode& currentElement)
{
  TiXmlElement* e = currentElement.ToElement();
  assert(e != nullptr);
  const char* a = e->Attribute(data.name);
  readInto(a, data, XmlDeserializerDomBase<detail::TinyDom>::ErrorContext(*e));
  e->RemoveAttribute(data.name);
}

template<typename DT>
inline
void TinyDom::extractAndDeleteSimpleChild(const NamedMemberForDeserialization<DT>& data, TiXmlNode& currentElement)
{
  TiXmlElement* e = currentElement.FirstChildElement(data.name);
  TiXmlNode*    n = (e == nullptr) ? nullptr : e->FirstChild();
  TiXmlText*    t = (n == nullptr) ? nullptr : n->ToText();
  readInto((t == nullptr) ? nullptr : t->Value(),
           data, t != nullptr ? XmlDeserializerDomBase<detail::TinyDom>::ErrorContext(*t)
                              : XmlDeserializerDomBase<detail::TinyDom>::ErrorContext(currentElement));
  currentElement.RemoveChild(e);
}

template<typename DT>
inline
void TinyDom::extractAndDeleteSingleChild(const NamedMemberForDeserialization<DT>& data, TiXmlNode& currentElement) {
  TiXmlNode* n = currentElement.FirstChild();
  TiXmlText* t = (n == nullptr) ? nullptr : n->ToText();
  readInto((t == nullptr) ? nullptr : t->Value(), data,
           t != nullptr ? XmlDeserializerDomBase<detail::TinyDom>::ErrorContext(*t)
                        : XmlDeserializerDomBase<detail::TinyDom>::ErrorContext(currentElement));
  currentElement.RemoveChild(t);
}

inline
TiXmlNode* TinyDom::getParentNode(TiXmlNode& currentElement) {
  return currentElement.Parent();
}

inline
void TinyDom::deleteChildNode(TiXmlNode& parentNode, TiXmlNode& childNode) {
  parentNode.RemoveChild(&childNode);
}

} // namespace detail

/**
 * \brief Deserializer of type sergut::detail::XmlDeserializerDomBase that uses TinyXML version 1
 * \see sergut::detail::XmlDeserializerDomBase
 */
typedef detail::XmlDeserializerDomBase<detail::TinyDom> XmlDeserializerTiny;

} // namespace sergut
