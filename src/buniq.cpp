
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

#define MAX_LINE_SIZE 256

void pin_to_current_cpu() {
  int cpu_id = sched_getcpu(); // 获取当前运行 CPU

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_id, &cpuset);

  int ret = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
  if (ret != 0) {
    perror("pthread_setaffinity_np");
  } else {
    printf("Pinned to CPU %d\n", cpu_id);
  }
}

int main(int argc, char *argv[]) {
  static_assert(sizeof(block_t) == 64, "block_t must be 64 bytes");
  static_assert(alignof(block_t) == 64, "block_t must be 64-byte aligned");
  // size_t cardinality = 8;
  // size_t precision = 6;
  // std::random_device rd;
  // size_t seed = (static_cast<size_t>(rd()) << 32) | rd();
  //
  // int opt;
  // while ((opt = getopt(argc, argv, "c:p:s:")) != -1) {
  //   switch (opt) {
  //   case 'c':
  //     cardinality = std::strtoul(optarg, nullptr, 10);
  //     break;
  //   case 'p':
  //     precision = std::strtoul(optarg, nullptr, 10);
  //     break;
  //   case 's':
  //     seed = std::strtoul(optarg, nullptr, 10);
  //     break;
  //   }
  // }
  //
  // bloom_filter_t bloom_filter{cardinality, precision, seed};

  // producer dispatch
  FILE *fp = stdin;
  if (optind < argc)
    fp = fopen(argv[optind], "r");
  if (!fp)
    throw std::runtime_error("Open File Failed");

  char buf[BUFSIZ];
  char line[MAX_LINE_SIZE];
  std::memset(buf, 0, sizeof(buf));
  std::memset(line, 0, sizeof(line));

  size_t n;
  size_t line_off = 0;
  while ((n = fread(buf, 1, MAX_LINE_SIZE, fp)) > 0) {
    size_t buf_off = 0;
    while (buf_off < n) {
      // to write pos: line + line_off
      char *nl = (char *)memchr(buf + buf_off, '\n', n - buf_off);
      if (!nl) { // the last incomplete line
        memcpy(line + line_off, buf + buf_off, n - buf_off);
        line_off += n - buf_off;
        break;
      }
      size_t chunk_size = (nl - (buf + buf_off)) + 1; // for \n
      memcpy(line + line_off, buf + buf_off, chunk_size);
      line_off += chunk_size;
      if (line_off > MAX_LINE_SIZE) [[unlikely]]
        throw std::runtime_error("Line Too Long");
      printf("%.*s", (int)line_off, line);
      line_off = 0;
      buf_off += chunk_size;
    }
  }
  if (line_off > 0) {
    printf("%.*s", (int)line_off, line);
  }
  // while (fgets(buf, sizeof(buf), fp)) {
  //   size_t len = strlen(buf);
  //   if (buf[len - 1] != '\n' && !feof(fp)) [[unlikely]]
  //     throw std::runtime_error("Line Too Long");
  //
  //   // if (bloom_filter.insert_if_new(buf, len)) { // new
  //   //   printf("%.*s", (int)len, buf);
  //   // }
  // }
  fclose(fp);
  return 0;
}
