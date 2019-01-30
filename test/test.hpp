#pragma once

#include "common.hpp"

void failTest(int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    fail("Test failed: failTest @ ", file, ":", line);
}

template <typename T>
void checkEqual(T a, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!(bool)(a == b)) {
        fail("Test failed: checkEqual(", a, ", ", b, ") @ ", file, ":", line);
    }
}

template <typename T>
void checkNotEqual(T a, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!(bool)(a != b)) {
        fail("Test failed: checkNotEqual(", a, ", ", b, ") @ ", file, ":", line);
    }
}

void checkTrue(bool val, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!val) {
        fail("Test failed: checkTrue(false) @ ", file, ":", line);
    }
}

void checkFalse(bool val, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(val) {
        fail("Test failed: checkFalse(true) @ ", file, ":", line);
    }
}

template <typename T>
void checkLessOrEqual(T a, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!(bool)(a <= b)) {
        fail("Test failed: checkLessOrEqual(", a, ", ", b, ") @ ", file, ":", line);
    }
}

template <typename T>
void checkLess(T a, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!(bool)(a < b)) {
        fail("Test failed: checkLess(", a, ", ", b, ") @ ", file, ":", line);
    }
}

template <typename T>
void checkGreaterOrEqual(T a, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!(bool)(a >= b)) {
        fail("Test failed: checkGreaterOrEqual(", a, ", ", b, ") @ ", file, ":", line);
    }
}

template <typename T>
void checkGreater(T a, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!(bool)(a > b)) {
        fail("Test failed: checkGreater(", a, ", ", b, ") @ ", file, ":", line);
    }
}

template <typename T>
void checkBetweenInclusive(T a, T x, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!((bool)(a <= x) && (bool)(x <= b))) {
        fail("Test failed: checkBetweenInclusive(", a, ", ", x, ", ", b, ") @ ", file, ":", line);
    }
}

template <typename T>
void checkBetweenExclusive(T a, T x, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!((bool)(a < x) && (bool)(x < b))) {
        fail("Test failed: checkBetweenExclusive(", a, ", ", x, ", ", b, ") @ ", file, ":", line);
    }
}

template <typename T>
void checkBetweenHalfOpen(T a, T x, T b, int line = __builtin_LINE(), const char* file = __builtin_FILE()) {
    if(!((bool)(a <= x) && (bool)(x < b))) {
        fail("Test failed: checkBetweenHalfOpen(", a, ", ", x, ", ", b, ") @ ", file, ":", line);
    }
}
