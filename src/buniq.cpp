
#include <cstddef>
#include <fcntl.h>
#include <memory>
#include <random>
#include <stdexcept>

#include <cstdio>
#include <unistd.h>

#include "atomic_queue.h"
#include "bloom_filter.hpp"
#include "murmur3.h"
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>

int main(int argc, char *argv[]) {
  size_t nthreads = std::thread::hardware_concurrency() - 1;
  if (nthreads <= 0)
    return 1;
  size_t scale = 8;
  size_t precision = 6;
  std::random_device rd;
  size_t seed = (static_cast<size_t>(rd()) << 32) | rd();

  int opt;
  while ((opt = getopt(argc, argv, "n:s:p:i:")) != -1) {
    switch (opt) {
    case 'n':
      nthreads = std::strtoul(optarg, nullptr, 10);
      break;
    case 's':
      scale = std::strtoul(optarg, nullptr, 10);
      break;
    case 'p':
      precision = std::strtoul(optarg, nullptr, 10);
      break;
    case 'i':
      seed = std::strtoul(optarg, nullptr, 10);
      break;
    }
  }

  bloom_filter_t bloom_filter{scale, precision, seed};

  std::vector<spsc_queue_t> queues(nthreads);
  std::vector<std::thread> workers;

  // create queues + workers
  for (size_t i = 0; i < nthreads; ++i) {
    workers.emplace_back([&bloom_filter, &queues, i]() {
      item_t item;
      size_t c = 0;
      while (true) {
        if (queues[i].pop(item)) {
          if (item.len == 0)
            break;                                // EOF signal
          if (bloom_filter.insert_if_new(item)) { // new
            printf("%.*s", (int)item.len, item.data);
          }
          c++;
        }
        std::this_thread::yield();
      }
      printf("worker %zu: %zu\n", i, c);
    });
  }

  // producer dispatch
  FILE *fp = stdin;
  if (optind < argc)
    fp = fopen(argv[optind], "r");
  if (!fp)
    throw std::runtime_error("Open File Failed");

  char buf[MAX_LINE_SIZE];
  std::memset(buf, 0, sizeof(buf));
  while (fgets(buf, sizeof(buf), fp)) {
    size_t len = strlen(buf);
    if (buf[len - 1] != '\n' && !feof(fp))
      throw std::runtime_error("Line Too Long");
    item_t item;
    item.len = len;
    memcpy(item.data, buf, len);
    // rand 分发
    while (!queues[std::rand() % nthreads].push(item)) {
      std::this_thread::yield();
    }
  }
  // IMPORTANT: close all write ends → send EOF
  for (size_t i = 0; i < nthreads; ++i) {
    item_t eof{};
    eof.len = 0;
    while (!queues[i].push(eof)) {
      std::this_thread::yield();
    }
  }

  // wait workers
  for (auto &t : workers)
    t.join();
  fclose(fp);
  return 0;
}
