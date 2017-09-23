#include <Arduino.h>

#ifndef _SIZE_H
#define _SIZE_H

template<typename T, size_t sz>
size_t size(T(&)[sz]) {
    return sz;
}

#endif
