#include "bloom_filter.h"

#include <stdlib.h>
#include <string.h>

uint64_t calc_hash(const char* str, uint64_t modulus, uint64_t seed) {
  uint64_t result = 0;

  for (uint64_t i = 0; str[i] != '\0'; ++i) {
    result *= seed;
    result += str[i];
    result %= modulus;
  }
  return result;
}

void bloom_init(struct BloomFilter* bloom_filter, uint64_t set_size,
                hash_fn_t hash_fn, uint64_t hash_fn_count) {
  bloom_filter->hash_fn = hash_fn;
  bloom_filter->hash_fn_count = hash_fn_count;
  bloom_filter->set_size = set_size;
  bloom_filter->set =
      malloc((set_size / sizeof(uint64_t) + 1) * sizeof(uint64_t));
  memset(bloom_filter->set, 0, set_size);
}

void bloom_destroy(struct BloomFilter* bloom_filter) {
  if (bloom_filter->set_size > 0) {
    free(bloom_filter->set);
  }
  bloom_filter->set = NULL;
}

void bloom_insert(struct BloomFilter* bloom_filter, Key key) {
  uint64_t* fns = malloc(sizeof(uint64_t) * bloom_filter->hash_fn_count);
  for (uint64_t i = 1; i <= bloom_filter->hash_fn_count; ++i) {
    fns[i - 1] =
        (*bloom_filter->hash_fn)(key, bloom_filter->set_size, 5 * i + 1);
  }

  for (size_t i = 0; i < bloom_filter->hash_fn_count; ++i) {
    bloom_filter->set[fns[i] / sizeof(uint64_t)] |=
        (1 << (fns[i] % sizeof(uint64_t)));
  }

  free(fns);
}

bool bloom_check(struct BloomFilter* bloom_filter, Key key) {
  uint64_t* fns = malloc(sizeof(uint64_t) * bloom_filter->hash_fn_count);
  for (uint64_t i = 1; i <= bloom_filter->hash_fn_count; ++i) {
    fns[i - 1] =
        (*bloom_filter->hash_fn)(key, bloom_filter->set_size, 5 * i + 1);
  }

  for (size_t i = 0; i < bloom_filter->hash_fn_count; ++i) {
    if ((bloom_filter->set[fns[i] / sizeof(uint64_t)] &
         (1 << (fns[i] % sizeof(uint64_t)))) == 0) {
      free(fns);
      return false;
    }
  }

  free(fns);
  return true;
}
