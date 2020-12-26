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

#include "sergut/JsonDeserializerBase.h"
#include "sergut/TypeTraits.h"

#include <QList>
#include <QSet>
#include <QVariant>
#include <QVector>

namespace sergut {

/**
 * \brief Specialization of deserializer for Qt classes.
 */
class JsonDeserializerQt: public JsonDeserializerBase<JsonDeserializerQt>
{
public:
  JsonDeserializerQt(const std::string& json);

  using JsonDeserializerBase::deserializeValue;

  // String types
  void deserializeValue(QString& data) {
    if(!_currentElement->IsString()) {
      throw ParsingException("Expected String");
    }
    data = (*_currentElement).GetString();
  }

  template<typename CDT>
  static void insertIntoCollection(QList<CDT>& collection, CDT&& data) {
    collection.insert(std::move(data));
  }

  template<typename CDT>
  static void insertIntoCollection(QSet<CDT>& collection, CDT&& data) {
    collection.insert(std::move(data));
  }

  template<typename CDT>
  static void insertIntoCollection(QVector<CDT>& collection, CDT&& data) {
    collection.push_back(std::move(data));
  }

  template<typename CDT>
  void deserializeValue(QList<CDT>& data) {
    deserializeCollection(data);
  }

  template<typename CDT>
  void deserializeValue(QSet<CDT>& data) {
    deserializeCollection(data);
  }

  template<typename CDT>
  void deserializeValue(QVector<CDT>& data) {
    deserializeCollection(data);
  }

  void deserializeValue(QVariant& data) {
    if(_currentElement->IsString()) {
      data = _currentElement->GetString();
    }
    else if(_currentElement->IsBool()) {
      data = _currentElement->GetBool();
    }
    else if(_currentElement->IsInt64()) {
      data = QVariant::fromValue(static_cast<long long>(_currentElement->GetInt64()));
    }
    else if(_currentElement->IsDouble()) {
      data = _currentElement->GetDouble();
    }
    else if(_currentElement->IsNull()) {
      data.clear();
    }
    else {
      throw new sergut::ParsingException("Type unsupported for QVariant");
    }
  }
};

template<>
struct is_serialization_format<JsonDeserializerQt, JsonFormat>: public std::true_type { };

} // namespace sergut
