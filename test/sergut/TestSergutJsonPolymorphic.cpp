#include <catch2/catch.hpp>

#include "sergut/JsonDeserializerQt.h"

#include <QVariant>

#include <typeindex>
struct TypesBase {
    virtual ~TypesBase() = default;
    virtual bool operator==(const TypesBase&) const = 0;
};

struct Type1: public TypesBase {
    int mynum1 = 17;
    bool mybool1 = true;

    Type1() = default;
    Type1(const Type1&) = default;
    Type1(int mynum, bool mybool)
      : mynum1(mynum)
      , mybool1(mybool)
    { }

    bool operator==(const TypesBase& rhs) const override;
};
bool operator==(const Type1& lhs, const Type1& rhs) {
  return lhs.mynum1 == rhs.mynum1
      && lhs.mybool1 == rhs.mybool1;
}
bool Type1::operator==(const TypesBase& rhs) const {
  const Type1* rhsPtr = dynamic_cast<const Type1*>(&rhs);
  if(rhsPtr == nullptr) {
    return false;
  }
  return *this == *rhsPtr;
}
SERGUT_FUNCTION(Type1, data, ar) {
    ar & SERGUT_MMEMBER(data, mynum1)
       & SERGUT_MMEMBER(data, mybool1);
}

struct Type2: TypesBase {
    std::string mystring2;
    Type1 myType1;

    Type2() = default;
    Type2(const Type2&) = default;
    Type2(std::string mystring, Type1 myType)
      : mystring2(std::move(mystring))
      , myType1(std::move(myType))
    { }

    bool operator==(const TypesBase& rhs) const override;
};
bool operator==(const Type2& lhs, const Type2& rhs) {
  return lhs.mystring2 == rhs.mystring2
      && lhs.myType1 == rhs.myType1;
}
bool Type2::operator==(const TypesBase& rhs) const {
  const Type2* rhsPtr = dynamic_cast<const Type2*>(&rhs);
  if(rhsPtr == nullptr) {
    return false;
  }
  return *this == *rhsPtr;
}
SERGUT_FUNCTION(Type2, data, ar) {
    ar & SERGUT_MMEMBER(data, mystring2)
       & SERGUT_MMEMBER(data, myType1);
}

struct Type3: TypesBase {
  std::string uniqueKey;
  int zahl;

  Type3() = default;
  Type3(const Type3&) = default;
  Type3(std::string myUniqueKey, int z)
    : uniqueKey(std::move(myUniqueKey))
    , zahl(z)
  { }

  bool operator==(const TypesBase& rhs) const override;
};
bool operator==(const Type3& lhs, const Type3& rhs) {
return lhs.uniqueKey == rhs.uniqueKey
    && lhs.zahl == rhs.zahl;
}
bool Type3::operator==(const TypesBase& rhs) const {
  const Type3* rhsPtr = dynamic_cast<const Type3*>(&rhs);
  if(rhsPtr == nullptr) {
    return false;
  }
  return *this == *rhsPtr;
}

SERGUT_FUNCTION(Type3, data, ar) {
  ar & SERGUT_MMEMBER(data, uniqueKey)
      & SERGUT_MMEMBER(data, zahl);
}

std::type_index getInternalTypeIndex(const std::unique_ptr<TypesBase>& c) {
    if(c == nullptr) {
        return std::type_index(typeid(void));
    }
    return std::type_index(typeid(c.get()));
}

struct Type1SerializationHandlerPoly {
    void init(std::unique_ptr<TypesBase>& p) {
        p.reset(new Type1{});
    }
    Type1& forwardToSubtype(std::unique_ptr<TypesBase>& ptr) {
        return static_cast<Type1&>(*ptr);
    }
    const Type1& forwardToSubtype(const std::unique_ptr<TypesBase>& ptr) {
      return static_cast<const Type1&>(*ptr);
    }
};

struct Type2SerializationHandlerPoly {
  void init(std::unique_ptr<TypesBase>& p) {
    p.reset(new Type2{});
  }
  Type2& forwardToSubtype(std::unique_ptr<TypesBase>& ptr) {
    return static_cast<Type2&>(*ptr);
  }
  const Type2& forwardToSubtype(const std::unique_ptr<TypesBase>& ptr) {
    return static_cast<const Type2&>(*ptr);
  }
};

struct Type3SerializationHandlerPoly {
  void init(std::unique_ptr<TypesBase>& p) {
    p.reset(new Type3{});
  }
  Type3& forwardToSubtype(std::unique_ptr<TypesBase>& ptr) {
    return static_cast<Type3&>(*ptr);
  }
  const Type3& forwardToSubtype(const std::unique_ptr<TypesBase>& ptr) {
    return static_cast<const Type3&>(*ptr);
  }
};

SERGUT_FUNCTION(std::unique_ptr<TypesBase>, data, ar) {
    ar.template oneOf(data, &getInternalTypeIndex,
                      Archive::template objectByKeyValue("serializationKey", "type1", std::type_index(typeid(Type1)), Type1SerializationHandlerPoly{}),
                      Archive::template objectByKeyValue("serializationKey", "type2", std::type_index(typeid(Type2)), Type2SerializationHandlerPoly{}),
                      Archive::template objectByKey("uniqueKey", std::type_index(typeid(Type3)), Type3SerializationHandlerPoly{}));
}

struct PolyPoly {
    std::unique_ptr<TypesBase> _p;
};

bool operator==(const PolyPoly& lhs, const PolyPoly& rhs) {
  if(lhs._p == nullptr) {
    if(rhs._p == nullptr) {
      return true;
    }
    return false;
  }
  if(rhs._p == nullptr) {
    return false;
  }
  return *lhs._p == *rhs._p;
}
SERGUT_FUNCTION(PolyPoly, data, ar) {
    ar & SERGUT_MMEMBER(data, _p);
}

TEST_CASE("Test deserialization of polymorphic type", "[sergut]") {
  GIVEN("A JSON with polymorphic element data types and Type1") {
    const std::string typeString = "{\"_p\":{\"serializationKey\":\"type1\",\"mynum1\":99,\"mybool1\":false}}";
    const PolyPoly referenceObject{ std::unique_ptr<TypesBase>(new Type1{99, false})};
    WHEN("Deserializing a Type1") {
      sergut::JsonDeserializerQt deser(typeString);
      const PolyPoly e = deser.deserializeData<PolyPoly>();
      THEN("The result is the specified string 1") {
        CHECK(e == referenceObject);
      }
    }
  }
  GIVEN("A JSON with polymorphic element data types and Type2") {
    const std::string typeString = "{\"_p\":{\"serializationKey\":\"type2\",\"mystring2\":\"baba\",\"myType1\":{\"mynum1\":99,\"mybool1\":false}}}";
    const PolyPoly referenceObject{ std::unique_ptr<TypesBase>(new Type2{"baba", Type1{99, false}})};
    WHEN("Deserializing a Type2") {
      sergut::JsonDeserializerQt deser(typeString);
      const PolyPoly e = deser.deserializeData<PolyPoly>();
      THEN("The result is the specified string 2") {
        CHECK(e == referenceObject);
      }
    }
  }
  GIVEN("A JSON with polymorphic element data types and Type3") {
    const std::string typeString = "{\"_p\":{\"uniqueKey\":\"lalala\",\"zahl\":17}}";
    const PolyPoly referenceObject{ std::unique_ptr<TypesBase>(new Type3{"lalala", 17})};
    WHEN("Deserializing a Type3") {
      sergut::JsonDeserializerQt deser(typeString);
      const PolyPoly e = deser.deserializeData<PolyPoly>();
      THEN("The result is the specified string 3") {
        CHECK(e == referenceObject);
      }
    }
  }
}

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
