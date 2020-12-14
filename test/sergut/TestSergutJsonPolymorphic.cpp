#include <catch2/catch.hpp>

#include "sergut/JsonDeserializerQt.h"

#include <QVariant>

struct VariantHolder {
  VariantHolder() = default;
  VariantHolder(QString dvor,
                QVariant vo,
                int dnach)
    : davor(dvor)
    , v(vo)
    , danach(dnach)
  { }
public:
  QString davor;
  QVariant v;
  int danach = 0;
};
bool operator==(const VariantHolder& lhs, const VariantHolder& rhs) {
  return lhs.davor == rhs.davor
      && lhs.v == rhs.v
      && lhs.danach == rhs.danach;
}
SERGUT_FUNCTION(VariantHolder, data, ar) {
  ar  & SERGUT_MMEMBER(data, davor)
      & SERGUT_OMEMBER(data, v)
      & SERGUT_OMEMBER(data, danach);
}

TEST_CASE("Test deserialization of QVariant", "[sergut]") {
  GIVEN("A JSON with variadic elementary data type (string)") {
    const std::string typeString = "{\"davor\":\"nase\",\"v\":\"test value\",\"danach\":77}";
    const VariantHolder refVal{"nase", QString("test value"), 77};
    WHEN("Deserializing a string") {
      sergut::JsonDeserializerQt deser(typeString);
      const VariantHolder e = deser.deserializeData<VariantHolder>();
      THEN("The result is the specified string") {
        CHECK(e == refVal);
      }
    }
  }
  GIVEN("A JSON with variadic elementary data type (integer)") {
    const std::string typeString = "{\"davor\":\"nase\",\"danach\":77,\"v\":4711}";
    const VariantHolder refVal{"nase", 4711, 77};
    WHEN("Deserializing a integer") {
      sergut::JsonDeserializerQt deser(typeString);
      const VariantHolder e = deser.deserializeData<VariantHolder>();
      THEN("The result is the specified integer") {
        CHECK(e == refVal);
      }
    }
  }
  GIVEN("A JSON with variadic elementary data type (double)") {
    const std::string typeString = "{\"davor\":\"nase\",\"danach\":77,\"v\":23.5}";
    const VariantHolder refVal{"nase", 23.5, 77};
    WHEN("Deserializing a double") {
      sergut::JsonDeserializerQt deser(typeString);
      const VariantHolder e = deser.deserializeData<VariantHolder>();
      THEN("The result is the specified double") {
        CHECK(e == refVal);
      }
    }
  }
  GIVEN("A JSON with variadic elementary data type (bool)") {
    const std::string typeString = "{\"v\":true,\"davor\":\"nase\",\"danach\":77}";
    const VariantHolder refVal{"nase", true, 77};
    WHEN("Deserializing a bool") {
      sergut::JsonDeserializerQt deser(typeString);
      const VariantHolder e = deser.deserializeData<VariantHolder>();
      THEN("The result is the specified bool") {
        CHECK(e == refVal);
      }
    }
  }
  GIVEN("A JSON with variadic elementary data type (null)") {
    const std::string typeString = "{\"v\":null,\"davor\":\"nase\",\"danach\":77}";
    const VariantHolder refVal{"nase", QVariant{}, 77};
    WHEN("Deserializing a null") {
      sergut::JsonDeserializerQt deser(typeString);
      const VariantHolder e = deser.deserializeData<VariantHolder>();
      THEN("The result is the specified null") {
        CHECK(e == refVal);
      }
    }
  }
}
