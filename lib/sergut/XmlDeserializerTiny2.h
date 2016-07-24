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

#include <tinyxml2.h>

#include <cassert>

namespace sergut {
namespace detail {

struct Tiny2Dom {
  typedef tinyxml2::XMLDocument Document;
  typedef tinyxml2::XMLNode Node;

  template<typename DT>
  static void extractAndDeleteAttribute(const NamedMemberForDeserialization<DT>& data, tinyxml2::XMLNode& currentElement);

  template<typename DT>
  static void extractAndDeleteSimpleChild(const NamedMemberForDeserialization<DT>& data, tinyxml2::XMLNode& currentElement);

  template<typename DT>
  static void extractAndDeleteSingleChild(const NamedMemberForDeserialization<DT>& data, tinyxml2::XMLNode& currentElement);

  static tinyxml2::XMLNode* getParentNode(tinyxml2::XMLNode& currentElement);

  static void deleteChildNode(tinyxml2::XMLNode& parentNode, tinyxml2::XMLNode& childNode);
};

template<>
class XmlDeserializerDomBase<detail::Tiny2Dom>::ErrorContext: public ParsingException::ErrorContext {
public:
  // tinyxml2 does not count node positions
  ErrorContext(const tinyxml2::XMLNode&) { }
  std::size_t getRow() const override { return std::size_t(-1); }
  std::size_t getColumn() const override { return std::size_t(-1); }
};

template<typename DT>
inline
void Tiny2Dom::extractAndDeleteAttribute(const NamedMemberForDeserialization<DT>& data, tinyxml2::XMLNode& currentElement) {
  tinyxml2::XMLElement* e = currentElement.ToElement();
  assert(e != nullptr);
  const char* a = e->Attribute(data.name);
  readInto(a, data, XmlDeserializerDomBase<detail::Tiny2Dom>::ErrorContext(*e));
  e->DeleteAttribute(data.name);
}

template<typename DT>
inline
void Tiny2Dom::extractAndDeleteSimpleChild(const NamedMemberForDeserialization<DT>& data, tinyxml2::XMLNode& currentElement) {
  tinyxml2::XMLElement* e = currentElement.FirstChildElement(data.name);
  tinyxml2::XMLNode*    n = (e == nullptr) ? nullptr : e->FirstChild();
  tinyxml2::XMLText*    t = (n == nullptr) ? nullptr : n->ToText();
  readInto((t == nullptr) ? nullptr : t->Value(),
           data, t != nullptr ? XmlDeserializerDomBase<detail::Tiny2Dom>::ErrorContext(*t)
                              : XmlDeserializerDomBase<detail::Tiny2Dom>::ErrorContext(currentElement));
  currentElement.DeleteChild(e);
}

template<typename DT>
inline
void Tiny2Dom::extractAndDeleteSingleChild(const NamedMemberForDeserialization<DT>& data, tinyxml2::XMLNode& currentElement) {
  tinyxml2::XMLNode* n = currentElement.FirstChild();
  tinyxml2::XMLText* t = (n == nullptr) ? nullptr : n->ToText();
  readInto((t == nullptr) ? nullptr : t->Value(),
           data, t != nullptr ? XmlDeserializerDomBase<detail::Tiny2Dom>::ErrorContext(*t)
                              : XmlDeserializerDomBase<detail::Tiny2Dom>::ErrorContext(currentElement));
  currentElement.DeleteChild(t);
}

inline
tinyxml2::XMLNode* Tiny2Dom::getParentNode(tinyxml2::XMLNode& currentElement) {
  return currentElement.Parent();
}

inline
void Tiny2Dom::deleteChildNode(tinyxml2::XMLNode& parentNode, tinyxml2::XMLNode& childNode) {
  parentNode.DeleteChild(&childNode);
}

} // namespace detail

typedef detail::XmlDeserializerDomBase<detail::Tiny2Dom> XmlDeserializerTiny2;

} // namespace sergut
