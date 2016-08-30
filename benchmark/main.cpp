#include "sergut/Util.h"
#include "sergut/XmlDeserializer.h"
#include "sergut/XmlDeserializerTiny.h"
#include "sergut/XmlDeserializerTiny2.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <random>

static const std::size_t repeat = 10;

struct NestingLevel3 {
  std::string string1;
  std::string string2;
  std::string string3;
  std::string string4;
  std::string string5;
};

SERGUT_FUNCTION(NestingLevel3, data, ar)
{
  ar &
      SERGUT_MMEMBER(data, string1) &
      SERGUT_MMEMBER(data, string2) &
      SERGUT_OMEMBER(data, string3) &
      SERGUT_MMEMBER(data, string4) &
      sergut::plainChild &
      SERGUT_MMEMBER(data, string5);
}

struct NestingLevel2 {
  std::string string1;
  std::vector<NestingLevel3> valuesLevel3;
  std::string string2;
};

SERGUT_FUNCTION(NestingLevel2, data, ar)
{
  ar &
      SERGUT_MMEMBER(data, string1) &
      sergut::children &
      SERGUT_NESTED_MMEMBER(data, valuesLevel3, valueLevel3) &
      SERGUT_OMEMBER(data, string2);
}

struct NestingLevel1 {
  std::string string1;
  std::vector<NestingLevel2> valuesLevel2;
  std::vector<NestingLevel3> valuesLevel3;
  std::string string2;
};

SERGUT_FUNCTION(NestingLevel1, data, ar)
{
  ar &
      SERGUT_MMEMBER(data, string1) &
      sergut::children &
      SERGUT_NESTED_MMEMBER(data, valuesLevel2, valueLevel2) &
      SERGUT_MMEMBER(data, valuesLevel3) &
      SERGUT_OMEMBER(data, string2);
}

struct NestingLevel0 {
  std::string string1;
  std::vector<NestingLevel1> valuesLevel1;
  std::vector<NestingLevel2> valuesLevel2;
  std::vector<NestingLevel3> valuesLevel3;
  std::string string2;
};

SERGUT_FUNCTION(NestingLevel0, data, ar)
{
  ar &
      SERGUT_MMEMBER(data, string1) &
      sergut::children &
      SERGUT_NESTED_MMEMBER(data, valuesLevel1, valueLevel1) &
      SERGUT_NESTED_MMEMBER(data, valuesLevel2, valueLevel2) &
      SERGUT_MMEMBER(data, valuesLevel3) &
      SERGUT_OMEMBER(data, string2);
}

static const char alphabet[] = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}";

typedef std::linear_congruential_engine<uint8_t, 17, 1, sizeof(alphabet)-1> RNG;

std::string generateRandomString(RNG& rng)
{
  std::string s;
  static const std::map<char, std::string> entities{
    {'\'', "&apos;"},
    {'"',  "&quot;"},
    {'&',  "&amp;"},
    {'<',  "&lt;"},
    {'>',  "&gt;"}
  };

  const std::size_t size = rng();
  s.reserve(size*3/2);
  for(std::size_t i = 0; i < size; ++i) {
    char c = alphabet[rng()];
    const auto entityIt = entities.find(c);
    if(entityIt != entities.end()) {
      s.append(entityIt->second);
    } else {
      s.insert(s.end(), c);
    }
  }
  return s;
}

std::string generateLevel3(RNG& rng, const std::string& name)
{
  std::string ret;
  for(std::size_t i = 0; i < repeat; ++i) {
    ret +=
        "<" + name + " "
        "string1='" + generateRandomString(rng) + "' "
//        "string6='" + generateRandomString(rng) + "' "
        "string4='" + generateRandomString(rng) + "' "
        "string2='" + generateRandomString(rng) +
        "'>\n" + generateRandomString(rng) + "\n"
        "</" + name + ">\n";
  }
  return ret;
}

std::string generateLevel2(RNG& rng, const std::string& name)
{
  std::string ret;
  for(std::size_t i = 0; i < repeat; ++i) {
    ret +=
        "<" + name + " "
        "string1='" + generateRandomString(rng) + "'>\n" +
        "<string2>" + generateRandomString(rng) + "</string2>\n"
        "<valuesLevel3>" + generateLevel3(rng, "valueLevel3") + "</valuesLevel3>\n"
        "</" + name + ">\n";
  }
  return ret;
}

std::string generateLevel1(RNG& rng, const std::string& name)
{
  std::string ret;
  for(std::size_t i = 0; i < repeat; ++i) {
    ret +=
        "<" + name + " "
        "string1='" + generateRandomString(rng) + "'>\n" +
        "<string2>" + generateRandomString(rng) + "</string2>\n"
        "<valuesLevel2>" + generateLevel2(rng, "valueLevel2") + "</valuesLevel2>\n" +
        generateLevel3(rng, "valuesLevel3") +
        "</" + name + ">\n";
  }
  return ret;
}

std::string generateLevel0(RNG& rng)
{
  std::string ret;
  for(std::size_t i = 0; i < repeat; ++i) {
    ret +=
        "<valuesLevel0 "
        "string1='" + generateRandomString(rng) + "'>\n" +
        "<string2>" + generateRandomString(rng) + "</string2>\n" +
        "<valuesLevel1>" + generateLevel1(rng, "valueLevel1") + "</valuesLevel1>\n" +
        "<valuesLevel2>" + generateLevel2(rng, "valueLevel2") + "</valuesLevel2>\n" +
        generateLevel3(rng, "valuesLevel3") +
        "</valuesLevel0>\n";
  }
  return ret;
}


struct Timer {
  Timer(const char* name) : _name(name), startTime(std::chrono::high_resolution_clock::now()) { }
  ~Timer() {
    const std::chrono::milliseconds us = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime);
    std::cout << _name
              << ": Elapsed: "
              << us.count()
              << "us"
              << std::endl; }
  const char* _name;
  std::chrono::high_resolution_clock::time_point startTime;
};

int main()
{
  RNG generator(23);

  const std::string data = "<outer>\n" + generateLevel0(generator) + "</outer>\n";

  std::cout << "XML String Size: " << data.size() << std::endl;

  for(int i = 0; i < 20; ++i) {
    {
      Timer t("XmlDeserializer");
      sergut::XmlDeserializer deser{sergut::misc::ConstStringRef(data)};
      deser.deserializeNestedData<NestingLevel0>("outer", "valuesLevel0");
    }
  }
  for(int i = 0; i < 20; ++i) {
    {
      Timer t("XmlDeserializerTiny");
      sergut::XmlDeserializerTiny deser(data);
      deser.deserializeNestedData<NestingLevel0>("outer", "valuesLevel0");
    }
  }
  for(int i = 0; i < 20; ++i) {
    {
      Timer t("XmlDeserializerTiny2");
      sergut::XmlDeserializerTiny2 deser(data);
      deser.deserializeNestedData<NestingLevel0>("outer", "valuesLevel0");
    }
  }


//  std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

//  std::cout << data << std::endl;

//  std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();

//  std::chrono::nanoseconds time_span = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);

//  std::cout << "It took me " << time_span.count() << " ns.";
//  std::cout << std::endl;

  return 0;
}
