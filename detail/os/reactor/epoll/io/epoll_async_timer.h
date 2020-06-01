/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/impl/epoll_impl.h"
#include "os/impl/linux_timer_impl.h"
#include "os/impl/unix_file_impl.h"
#include "os/reactor/epoll/epoll_service.h"
#include "os/reactor/epoll/events/epoll_timer.h"

#include <mutex>

namespace baba::os {

class async_timer final {
 public:
  NOT_COPYABLE(async_timer)

  async_timer() = default;

  async_timer(async_timer &&tmp) noexcept
      : _strand_for_user(tmp._strand_for_user),
        _strand(tmp._strand),
        _descriptor(std::move(tmp._descriptor)),
        _timer(std::move(tmp._timer)) {
    tmp._strand_for_user = nullptr;
    tmp._strand = nullptr;
  }

  async_timer &operator=(async_timer &&tmp) noexcept {
    _strand_for_user = tmp._strand_for_user;
    _strand = tmp._strand;
    _descriptor = std::move(tmp._descriptor);
    _timer = std::move(tmp._timer);
    tmp._strand_for_user = nullptr;
    tmp._strand = nullptr;
    return *this;
  }

  async_timer(completion_strand &strand) noexcept
      : _strand_for_user(&strand),
        _strand(strand.impl()),
        _descriptor(new reactor_io_descriptor()),
        _timer(
            _descriptor, _strand, _strand->service().registrar(),
            _strand->service().deletion_queue(),
            [epoll_fd = _strand->service().reactor().fd()](reactor_event *evt,
                                                           uint32_t timeout_ms) {
              evt->descriptor->set_read_mask();
              const auto ec = epoll_impl::arm(epoll_fd, evt->descriptor->fd,
                                              evt->descriptor->event_mask, evt->descriptor.get());
              if (ec != ec::OK) {
                return ec;
              }
              return linux_timer_impl::init_timer(evt->descriptor->fd, timeout_ms);
            },
            [](io_handle fd, uint8_t *buffer, int size, int &bytes_read) {
              const auto [e, bytes] = unix_file_impl::read(fd, buffer, size);
              bytes_read = bytes;
              return e;
            }) {
    RUNTIME_ASSERT(_strand);
    const auto [e, fd] = linux_timer_impl::open();
    if (e == ec::OK) {
      _descriptor->fd = fd;
    }
  }

  ~async_timer() noexcept {
    // Check whether it is not default constructed or been moved
    if (_strand) {
      // When running `complete`, it is possible that the callback it execute will call this
      // destructor. For example, if this object is created as a shared_ptr and the callback
      // reset its own object, if this happens, it will be a deadlock since `complete` is waiting
      // for the destructor to complete but can't because they are locking the same mutex. To
      // prevent this deadlock, we use a recursive mutex instead.
      std::lock_guard lock(_descriptor->in_scope_mtx);
      if (_descriptor->in_scope) {
        _descriptor->in_scope = false;
        // Remove event from the reactor. Deletion event will handle its clean up
        epoll_impl::remove(_strand->service().reactor().fd(), _descriptor->fd);
        unix_file_impl::close(_descriptor->fd);
        _timer.dispose();
      }
    }
  }

  void timeout(uint32_t timeout_ms, const epoll_timer::expired_fn &cb) noexcept {
    _timer.timeout(timeout_ms, cb);
  }

  completion_strand &strand() noexcept {
    // This strand is only for users of this object but must not be used for operation as
    // this have shorter lifetime than the registrar and can cause issues during shutdown
    RUNTIME_ASSERT(_strand_for_user);
    return *_strand_for_user;
  }

  epoll_service &service() noexcept {
    RUNTIME_ASSERT(_strand);
    return _strand->service();
  }

  io_handle fd() const noexcept { return _descriptor->fd; }

 private:
  std::unique_ptr<std::mutex> _mtx;
  completion_strand *_strand_for_user = nullptr;
  completion_strand_impl *_strand = nullptr;
  reactor_io_descriptor_ptr _descriptor;
  epoll_timer _timer;
};

}  // namespace baba::os