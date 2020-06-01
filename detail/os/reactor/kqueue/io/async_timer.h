/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/impl/kqueue_timer_impl.h"
#include "os/impl/unix_file_impl.h"
#include "os/reactor/kqueue/events/kqueue_timer.h"
#include "os/reactor/kqueue/kqueue_service.h"

namespace baba::os {

class async_timer final {
 public:
  async_timer(const lifetime &scope, const kqueue_service &service) noexcept
      : _descriptor(new reactor_io_descriptor()),
        _timer(scope, _descriptor, service.initiation_queue(), service.reactor_reg(),
               service.completion_queue()) {
    _descriptor->fd = kqueue_timer_impl::create();
  }

  void timeout(uint32_t timeout_ms, const kqueue_timer::expired_fn &cb) noexcept {
    _timer.timeout(timeout_ms, cb);
  }

  void timeout() noexcept { _timer.timeout(); }

 private:
  std::shared_ptr<reactor_io_descriptor> _descriptor;
  kqueue_timer _timer;
};

}  // namespace baba::os