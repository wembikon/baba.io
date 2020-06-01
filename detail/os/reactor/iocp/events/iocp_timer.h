/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/reactor/iocp/iocp_reactor_event.h"

#include <mutex>

namespace baba::os {

class iocp_timer final {
 public:
  using expired_fn = std::function<void(error_code)>;
  using open_timer_fn = std::function<void(reactor_event *)>;
  using close_timer_fn = std::function<void(reactor_event *)>;
  using init_timer_fn = std::function<error_code(io_handle, uint32_t)>;

  iocp_timer(const lifetime &scope, const reactor_io_descriptor_ptr &io_desc,
             const enqueue_for_completion_fn &enqueue_for_completion,
             const enqueue_for_deletion_fn &enqueue_for_deletion, const open_timer_fn &open_timer,
             const close_timer_fn &close_timer, const init_timer_fn &init_timer) noexcept;
  ~iocp_timer() noexcept;
  void timeout(uint32_t timeout_ms, const expired_fn &cb) noexcept;

 private:
  std::mutex _mtx;
  reactor_event *_evt;
  enqueue_for_deletion_fn _enqueue_for_deletion;
  init_timer_fn _init_timer;
  close_timer_fn _close_timer;
};

}  // namespace baba::os