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

#include "sergut/SerializationException.h"
#include "sergut/JsonSerializerBase.h"
#include "sergut/TypeTraits.h"

#include <QList>
#include <QSet>
#include <QVariant>
#include <QVector>

namespace sergut {

/**
 * \brief Specialization of deserializer for Qt classes.
 */
class JsonSerializerQt: public JsonSerializerBase<JsonSerializerQt>
{
public:
  JsonSerializerQt(const Flags flags = Flags::BoolAsInt)
    : JsonSerializerBase(flags)
  { }

  using JsonSerializerBase::serializeValue;

  // String types
  void serializeValue(const QString& data) {
    serializeValue(data.toStdString());
  }

  template<typename CDT>
  void serializeValue(const QList<CDT>& data) {
    serializeCollection(data);
  }

  template<typename CDT>
  void serializeValue(const QSet<CDT>& data) {
    serializeCollection(data);
  }

  template<typename CDT>
  void serializeValue(const QVector<CDT>& data) {
    serializeCollection(data);
  }

  void serializeValue(const QVariant& data) {
    if(data.isNull()) {
      out() << "null";
      return;
    }
    switch(data.type()) {
    case QVariant::Bool:
      serializeValue(data.toBool());
      return;
    case QVariant::Char:
    case QVariant::String:
      serializeValue(data.toString());
      return;
    case QVariant::UInt:
    case QVariant::ULongLong:
      serializeValue(data.toULongLong());
      return;
    case QVariant::Int:
    case QVariant::LongLong:
      serializeValue(data.toLongLong());
      return;
    case QVariant::Double:
      serializeValue(data.toDouble());
      return;
    default:
      throw sergut::SerializationException("Unsupported QVariant Type");
    }
  }
};

template<>
struct is_serialization_format<JsonSerializerQt, JsonFormat>: public std::true_type { };

} // namespace sergut
