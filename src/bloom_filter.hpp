
#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H
#include "atomic_queue.h"
#include "murmur3.h"
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

const size_t bits_in_cacheline = 504;

struct block_t {
  std::atomic_flag spinlock;
  uint8_t data[63];

  block_t() {
    spinlock.clear(std::memory_order_release);
    std::memset(data, 0, 63);
  }
  void lock() {
    while (spinlock.test_and_set(std::memory_order_acquire))
      std::this_thread::yield();
  }
  void unlock() { spinlock.clear(std::memory_order_release); }
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

  bloom_filter_t(size_t scale, size_t precision, size_t seed) {
    double ln10 = std::log(10.0L);
    double ln2 = std::log(2.0L);
    double n = std::pow(10.0, scale);              // n = 10^scale
    double m = n * precision * ln10 / (ln2 * ln2); // m bits
    double k = precision * ln10 / ln2;             // k hashes
    this->numblock = (size_t)std::ceil(m / bits_in_cacheline);
    this->numhash = (size_t)std::round(k);
    this->blocks = std::vector<block_t>(numblock);
    this->seed = seed;
  }

  bool insert_if_new(const item_t &item) {
    size_t block_idx =
        murmur3_32((uint8_t *)item.data, item.len, seed) % numblock;
    size_t hash1 =
        murmur3_32((uint8_t *)item.data, item.len, seed + 1) % numblock;
    size_t hash2 =
        murmur3_32((uint8_t *)item.data, item.len, seed + 2) % numblock;
    blocks[block_idx].lock();
    bool is_new = false;

    for (size_t i = 0; i < this->numhash; ++i) {
      size_t local_idx = (hash1 + i * hash2) % bits_in_cacheline;
      if (!blocks[block_idx].get(local_idx))
        is_new = true;
    }
    if (is_new) {
      for (size_t i = 0; i < this->numhash; ++i) {
        size_t local_idx = (hash1 + i * hash2) % bits_in_cacheline;
        blocks[block_idx].set(local_idx);
      }
    }
    blocks[block_idx].unlock();
    return is_new;
  }
};

#endif
