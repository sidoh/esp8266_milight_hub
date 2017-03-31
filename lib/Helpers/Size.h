#include <Arduino.h>

template<typename T, size_t sz>
size_t size(T(&)[sz]) {
    return sz;
}