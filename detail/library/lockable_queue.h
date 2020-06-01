/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"

#include <deque>
#include <mutex>

namespace baba {

template <typename Type>
class lockable_queue final {
 public:
  lockable_queue() = default;

  void enqueue(const Type& e) noexcept {
    std::lock_guard lock(_mutex);
    _queue.emplace_back(e);
  }

  std::mutex& mtx() noexcept { return _mutex; }

  Type dequeue() noexcept {
    RUNTIME_ASSERT(_queue.size() > 0);
    const auto evt = _queue.front();
    _queue.pop_front();
    return evt;
  }

  auto size() noexcept { return _queue.size(); }

 private:
  std::mutex _mutex;
  std::deque<Type> _queue;
};

}  // namespace baba