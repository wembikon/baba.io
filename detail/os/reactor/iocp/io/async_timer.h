/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/impl/windows_timer_impl.h"
#include "os/reactor/iocp/events/iocp_timer.h"
#include "os/reactor/iocp/iocp_service.h"
#include "baba/semantics.h"

namespace baba::os {

class async_timer final {
 public:
  async_timer(const lifetime &scope, const iocp_service &service) noexcept
      : _descriptor(new reactor_io_descriptor()),
        _timer(scope, _descriptor, service.completion_queue(), service.deletion_queue(),
        [this, &service](reactor_event *evt) {
          RUNTIME_ASSERT(evt);
          _iocp_ctx.evt = evt;
          _iocp_ctx.iocp_fd = service.reactor().fd();
          const auto &[e, fd] = windows_timer_impl::open(&_iocp_ctx);
          if(e == ec::OK) {
            _descriptor->fd = fd;
          }
        },[](reactor_event *evt) {
          RUNTIME_ASSERT(evt);
          if(evt->descriptor) {
            windows_timer_impl::close(evt->descriptor->fd);
            evt->descriptor.reset();
          }
        },[](io_handle fd, uint32_t timeout_ms){
          return windows_timer_impl::init_timer(fd, timeout_ms);
        }) {}

  void timeout(uint32_t timeout_ms, const iocp_timer::expired_fn &cb) noexcept {
    _timer.timeout(timeout_ms, cb);
  }

 private:
  reactor_io_descriptor_ptr _descriptor;
  windows_timer_impl::iocp_timer_context _iocp_ctx;
  iocp_timer _timer;
};

}  // namespace baba::os