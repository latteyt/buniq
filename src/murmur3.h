
#ifndef MURMUR3_H
#define MURMUR3_H

#include <cstdint>
#include <cstring>

inline uint32_t murmur3_32(const uint8_t *key, size_t len, uint32_t seed) {
  uint32_t h = seed;
  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  const int nblocks = len / 4;
  const uint32_t *blocks = (const uint32_t *)(key);

  for (int i = 0; i < nblocks; i++) {
    uint32_t k = blocks[i];
    k *= c1;
    k = (k << 15) | (k >> 17);
    k *= c2;

    h ^= k;
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;
  }

  const uint8_t *tail = key + nblocks * 4;
  uint32_t k1 = 0;

  switch (len & 3) {
  case 3:
    k1 ^= tail[2] << 16;
  case 2:
    k1 ^= tail[1] << 8;
  case 1:
    k1 ^= tail[0];
    k1 *= c1;
    k1 = (k1 << 15) | (k1 >> 17);
    k1 *= c2;
    h ^= k1;
  }

  h ^= len;
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}
#endif
