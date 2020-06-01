/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"

#include <deque>
#include <mutex>
#include <optional>

namespace baba {

template <typename Type>
class safe_queue final {
 public:
  safe_queue() = default;

  void enqueue(const Type &e) noexcept {
    std::lock_guard lock(_mutex);
    _queue.emplace_back(e);
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

 private:
  std::mutex _mutex;
  std::deque<Type> _queue;
};

}  // namespace baba