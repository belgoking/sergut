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

template<typename Archive, typename DT>
struct has_serialize_function {
  using yesType = char;
  using noType = struct{ char x[2]; };
  using baseDT = typename std::decay<DT>::type;
  using baseArchive = typename std::decay<Archive>::type;
  template<typename A, typename T>
  static
  yesType test(decltype(serialize(declval<A&>(), declval<T&>(), static_cast<T*>(nullptr)), int()));
  template<typename A, typename T>
  static
  noType test(...);
  enum { value = sizeof(test<baseArchive, baseDT>(3)) == sizeof(yesType) };
};

}
