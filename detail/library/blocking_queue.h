/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace baba {

template <typename Type>
class blocking_queue final {
 public:
  blocking_queue() = default;

  void enqueue(const Type &e) noexcept {
    std::lock_guard lock(_mutex);
    _queue.emplace_back(e);
    _signaller.notify_all();
  }

  std::optional<Type> dequeue() noexcept {
    std::lock_guard lock(_mutex);
    if (_queue.size() > 0) {
      const auto evt = _queue.front();
      _queue.pop_front();
      return evt;
    }
    return std::nullopt;
  }

  auto size() noexcept { return _queue.size(); }

  void wait() noexcept {
    std::unique_lock lock(_mutex);
    while (_active && _queue.size() == 0) {
      _signaller.wait(lock);
    }
  }

  void stop() noexcept {
    std::unique_lock lock(_mutex);
    _active = false;
    _signaller.notify_all();
  }

  /**
   * Important! Only call this after all other threads are stopped that uses this blocking_queue
   * and only call this on the main thread for clean up purposes
   **/
  void clear() noexcept { _queue.clear(); }

 private:
  bool _active = true;
  std::deque<Type> _queue;
  std::mutex _mutex;
  std::condition_variable _signaller;
};

}  // namespace baba