#include <ArduinoJson.h>
#include <vector>
#include <functional>
#include <algorithm>

#ifndef _JSON_HELPERS_H
#define _JSON_HELPERS_H

class JsonHelpers {
public:
  template<typename T>
  static void copyFrom(JsonArray arr, std::vector<T> vec) {
    for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
      arr.add(*it);
    }
  }

  template<typename T>
  static void copyTo(JsonArray arr, std::vector<T> vec) {
    for (size_t i = 0; i < arr.size(); ++i) {
      JsonVariant val = arr[i];
      vec.push_back(val.as<T>());
    }
  }

  template<typename T, typename StrType>
  static std::vector<T> jsonArrToVector(JsonArray& arr, std::function<T (const StrType)> converter, const bool unique = true) {
    std::vector<T> vec;

    for (size_t i = 0; i < arr.size(); ++i) {
      StrType strVal = arr[i];
      T convertedVal = converter(strVal);

      // inefficient, but everything using this is tiny, so doesn't matter
      if (!unique || std::find(vec.begin(), vec.end(), convertedVal) == vec.end()) {
        vec.push_back(convertedVal);
      }
    }

    return vec;
  }

  template<typename T, typename StrType>
  static void vectorToJsonArr(JsonArray& arr, const std::vector<T>& vec, std::function<StrType (const T&)> converter) {
    for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
      arr.add(converter(*it));
    }
  }
};

#endif