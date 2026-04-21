#ifndef ATOMIC_QUEUE_H
#define ATOMIC_QUEUE_H
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>

#define MAX_LINE_SIZE 1024
#define RING_BUF_SIZE 128

struct item_t {
  uint32_t len;
  char data[MAX_LINE_SIZE];
};

struct spsc_queue_t {

  item_t buffer[RING_BUF_SIZE];
  std::atomic<size_t> head{0};
  std::atomic<size_t> tail{0};

  bool push(const item_t &item) {
    size_t h = head.load(std::memory_order_relaxed);
    size_t next = (h + 1) % RING_BUF_SIZE;

    if (next == tail.load(std::memory_order_acquire))
      return false; // full

    buffer[h] = item;
    head.store(next, std::memory_order_release);
    return true;
  }

  bool pop(item_t &item) {
    size_t t = tail.load(std::memory_order_relaxed);

    if (t == head.load(std::memory_order_acquire))
      return false; // empty

    item = buffer[t];
    tail.store((t + 1) % RING_BUF_SIZE, std::memory_order_release);
    return true;
  }
};

#endif
