
#include <cstddef>
#include <emmintrin.h>
#include <fcntl.h>
#include <pthread.h>
#include <random>
#include <sched.h>
#include <stdexcept>

#include <cstdio>
#include <unistd.h>

#include "bloom_filter.hpp"
#include <cstdlib>
#include <cstring>

constexpr size_t MAX_LINE_SIZE = 128;

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

  FILE *fp = stdin;
  if (optind < argc)
    fp = fopen(argv[optind], "r");
  if (!fp)
    throw std::runtime_error("Open File Failed");

  char buf[MAX_LINE_SIZE];
  std::memset(buf, 0, sizeof(buf));

  while (fgets(buf, sizeof(buf), fp)) {
    char *nl = (char *)memchr(buf, '\n', sizeof(buf));
    if (!nl) [[unlikely]]
      throw std::runtime_error("Line Too Long");

    size_t len = nl - buf + 1;
    if (bloom_filter.insert_if_new(buf, len)) // new
      fwrite(buf, 1, len, stdout);
  }
  fclose(fp);
  return 0;
}
