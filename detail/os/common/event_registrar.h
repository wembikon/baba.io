/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include <map>
#include <mutex>

namespace baba {

struct reactor_event;

class event_registrar final {
 public:
  event_registrar() = default;
  reactor_event* take() noexcept;
  void give(reactor_event* evt) noexcept;
  void close() noexcept;

 private:
  std::mutex _mutex;
  std::map<uint64_t, reactor_event> _register;
  uint64_t _id_generator = 0;
};

}  // namespace baba