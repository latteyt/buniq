
#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H
#include "murmur3.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <emmintrin.h>
#include <vector>

static constexpr size_t bits = 64;
struct alignas(64) block_t {
  uint8_t data[64];

  block_t() { std::memset(data, 0, 64); }
  void set(size_t idx) {
    size_t x = idx / 8;
    size_t y = idx % 8;
    data[x] |= (1 << y);
  }
  bool get(size_t idx) const {
    size_t x = idx / 8;
    size_t y = idx % 8;
    return data[x] & (1 << y);
  }
};

struct bloom_filter_t {
  std::vector<block_t> blocks;
  size_t numblock;
  size_t numhash;
  size_t seed;

  bloom_filter_t(size_t cardinality, size_t precision, size_t seed) {
    double ln10 = std::log(10.0L);
    double ln2 = std::log(2.0L);
    double n = std::pow(10.0, cardinality);
    double m = n * precision * ln10 / (ln2 * ln2);
    double k = precision * ln10 / ln2;
    this->numblock = (size_t)std::ceil(m / bits);
    this->numhash = (size_t)std::round(k);
    this->blocks = std::vector<block_t>(numblock);
    this->seed = seed;
  }

  bool insert_if_new(const char *buf, const size_t len) {
    size_t block_idx = murmur3_32((uint8_t *)buf, len, seed) % numblock;
    size_t hash1 = murmur3_32((uint8_t *)buf, len, seed + 1) % numblock;
    size_t hash2 = murmur3_32((uint8_t *)buf, len, seed + 2) % numblock;
    bool is_new = false;

    for (size_t i = 0; i < this->numhash; ++i) {
      size_t local_idx = (hash1 + i * hash2) % bits;
      if (!blocks[block_idx].get(local_idx))
        is_new = true;
    }
    if (is_new) {
      for (size_t i = 0; i < this->numhash; ++i) {
        size_t local_idx = (hash1 + i * hash2) % bits;
        blocks[block_idx].set(local_idx);
      }
    }
    return is_new;
  }
};

#endif
