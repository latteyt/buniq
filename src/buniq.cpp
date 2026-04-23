
#include <atomic>
#include <cstddef>
#include <emmintrin.h>
#include <fcntl.h>
#include <pthread.h>
#include <random>
#include <sched.h>
#include <stdexcept>

#include <cstdio>
#include <thread>
#include <unistd.h>

#include "bloom_filter.hpp"
#include <cstdlib>
#include <cstring>

constexpr size_t MAX_LINE_SIZE = 128;
constexpr size_t SLOT_SIZE = 4096 / MAX_LINE_SIZE;
static char slots[SLOT_SIZE][MAX_LINE_SIZE];
static std::atomic<size_t> head{0};
static std::atomic<size_t> tail{0};

int main(int argc, char *argv[]) {
  static_assert(sizeof(block_t) == 64, "block_t must be 64 bytes");
  static_assert(alignof(block_t) == 64, "block_t must be 64-byte aligned");
  size_t cardinality = 8;
  size_t precision = 6;
  std::random_device rd;
  size_t seed = (static_cast<size_t>(rd()) << 32) | rd();

  int opt;
  while ((opt = getopt(argc, argv, "c:p:s:")) != -1) {
    switch (opt) {
    case 'c':
      cardinality = std::strtoul(optarg, nullptr, 10);
      break;
    case 'p':
      precision = std::strtoul(optarg, nullptr, 10);
      break;
    case 's':
      seed = std::strtoul(optarg, nullptr, 10);
      break;
    }
  }

  bloom_filter_t bloom_filter{cardinality, precision, seed};

  std::thread worker([&] {
    while (true) {
      size_t t = tail.load(std::memory_order_relaxed);
      while (t == head.load(std::memory_order_acquire))
        _mm_pause(); // empty

      if (slots[t][0] == '\0')
        break;

      char *nl = (char *)memchr(slots[t], '\n', MAX_LINE_SIZE);
      if (!nl) [[unlikely]]
        throw std::runtime_error("Line Too Long");

      size_t len = nl - slots[t] + 1;
      if (bloom_filter.insert_if_new(slots[t], len)) // new
        fwrite(slots[t], 1, len, stdout);

      size_t next = (t + 1) % SLOT_SIZE;
      tail.store(next, std::memory_order_release);
    }
  });

  FILE *fp = stdin;
  if (optind < argc)
    fp = fopen(argv[optind], "r");
  if (!fp)
    throw std::runtime_error("Open File Failed");

  while (true) {
    size_t h = head.load(std::memory_order_relaxed);
    size_t next = (h + 1) % SLOT_SIZE;
    while (next == tail.load(std::memory_order_acquire))
      _mm_pause(); // full -> busy wait

    if (!fgets(slots[h], MAX_LINE_SIZE, fp)) {
      slots[h][0] = '\0';
      head.store(next, std::memory_order_release);
      break;
    }
    head.store(next, std::memory_order_release);
  }

  // char buf[MAX_LINE_SIZE];
  // std::memset(buf, 0, sizeof(buf));
  //
  // while (fgets(buf, sizeof(buf), fp)) {
  //   char *nl = (char *)memchr(buf, '\n', sizeof(buf));
  //   if (!nl) [[unlikely]]
  //     throw std::runtime_error("Line Too Long");
  //
  //   size_t len = nl - buf + 1;
  //   if (bloom_filter.insert_if_new(buf, len)) // new
  //     fwrite(buf, 1, len, stdout);
  // }
  fclose(fp);
  worker.join();
  return 0;
}
