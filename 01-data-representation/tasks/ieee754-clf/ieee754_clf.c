#include "ieee754_clf.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

float_class_t classify(double x) {
  uint64_t num;
  _Static_assert(sizeof(double) == sizeof(uint64_t), "sizeof missmatch :(");
  memcpy(&num, &x, sizeof(double));

  bool sign = num & (1ull << 63);
  uint16_t exp = (num & ((1ull << 63) - (1ull << 52))) >> 52;
  uint64_t mantissa = (num & ((1ull << 52) - 1));

  if (exp == 0 && mantissa == 0) {
    if (sign) {
      return MinusZero;
    }
    return Zero;
  }

  if (exp != 0 && exp != ((1 << 11) - 1)) {
    if (sign) {
      return MinusRegular;
    }
    return Regular;
  }

  if (exp == 0) {
    if (sign) {
      return MinusDenormal;
    }
    return Denormal;
  }

  if (mantissa == 0) {
    if (sign) {
      return MinusInf;
    }
    return Inf;
  }

  return NaN;
}
