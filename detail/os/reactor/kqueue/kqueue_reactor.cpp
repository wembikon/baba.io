/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/kqueue/kqueue_reactor.h"

#include "baba/logger.h"
#include "baba/semantics.h"
#include "os/impl/kqueue_impl.h"
#include "os/impl/unix_file_impl.h"

namespace baba::os {

kqueue_reactor::kqueue_reactor() noexcept {
  const auto [e, fd] = kqueue_impl::create();
  if (e != ec::OK) {
    LOGFTL("Cannot instantiate kqueue");
  }
  _fd = fd;
}

kqueue_reactor::~kqueue_reactor() noexcept { unix_file_impl::close(_fd); }

void kqueue_reactor::initiate(reactor_event *evt) noexcept {
  RUNTIME_ASSERT(evt);
  evt->initiate(_fd);
}

error_code kqueue_reactor::wait() noexcept {
  if (const auto [e, ready] = kqueue_impl::wait(_fd, _event_list, MAX_KQUEUE_EVENTS); e == ec::OK) {
    for (int i = 0; i < ready; ++i) {
      const auto evt = static_cast<reactor_event *>(_event_list[i].udata);
      if (evt->descriptor->fd == _interrupter_fd) {
        continue;
      }
      evt->react();
    }
    return ec::OK;
  } else {
    return e;
  }
}

io_handle kqueue_reactor::fd() noexcept { return _fd; }

void kqueue_reactor::set_interrupter_fd(io_handle fd) noexcept { _interrupter_fd = fd; }

}  // namespace baba::os