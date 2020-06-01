/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/reactor/kqueue/kqueue_reactor_event.h"

namespace baba::os {

class kqueue_timer final {
 public:
  using expired_fn = std::function<void(error_code)>;

  kqueue_timer(const lifetime &scope, const std::shared_ptr<reactor_io_descriptor> &io_desc,
               const enqueue_for_initiation_fn &enqueue_for_initiation,
               const register_to_reactor_fn &register_to_reactor,
               const enqueue_for_completion_fn &enqueue_for_completion) noexcept;
  ~kqueue_timer() noexcept;
  void timeout(uint32_t timeout_ms, const expired_fn &cb) noexcept;
  void timeout() noexcept;

 private:
  reactor_event *_evt;
};

}  // namespace baba::os