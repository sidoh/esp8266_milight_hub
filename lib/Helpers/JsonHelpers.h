#include <ArduinoJson.h>
#include <vector>
#include <functional>
#include <algorithm>

#ifndef _JSON_HELPERS_H
#define _JSON_HELPERS_H

class JsonHelpers {
public:
  template<typename T>
  static std::vector<T> jsonArrToVector(JsonArray& arr, std::function<T (const String&)> converter, const bool unique = true) {
    std::vector<T> vec;

    for (size_t i = 0; i < arr.size(); ++i) {
      String strVal = arr.get<const char*>(i);
      T convertedVal = converter(strVal);

      // inefficient, but everything using this is tiny, so doesn't matter
      if (!unique || std::find(vec.begin(), vec.end(), convertedVal) == vec.end()) {
        vec.push_back(convertedVal);
      }
    }

    return vec;
  }

  template<typename T>
  static void vectorToJsonArr(JsonArray& arr, const std::vector<T>& vec, std::function<String (const T&)> converter) {
    for (typename std::vector<T>::const_iterator it = vec.begin(); it != vec.end(); ++it) {
      arr.add(converter(*it));
    }
  }
};

#endif