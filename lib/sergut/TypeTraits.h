#pragma once

#include <type_traits>

namespace sergut {

template<typename Archive, typename SerializationFormat>
struct is_serialization_format: public std::false_type { };

struct JsonFormat { };

struct XmlFormat { };

struct UrlFormat { };

/**
 * \brief Helper template function for SFINAE similar to \c std::declval()
 */
template<class T>
T declval() noexcept;

}
