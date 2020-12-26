#pragma once

#include <type_traits>

namespace sergut {

template<typename Archive, typename SerializationFormat>
struct is_serialization_format: public std::false_type { };

struct JsonFormat { };

struct XmlFormat { };

struct UrlFormat { };

}
