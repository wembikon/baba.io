/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/events/epoll_send_to.h"

#include "os/common/event_registrar.h"

namespace baba::os {

epoll_send_to::epoll_send_to(epoll_send_to &&tmp) noexcept
    : _mtx(std::move(tmp._mtx)),
      _evt(tmp._evt),
      _enqueue_for_deletion(std::move(tmp._enqueue_for_deletion)),
      _init_send_to(std::move(tmp._init_send_to)),
      _fin_send_to(std::move(tmp._fin_send_to)) {
  tmp._evt = nullptr;
}

epoll_send_to &epoll_send_to::operator=(epoll_send_to &&tmp) noexcept {
  _mtx = std::move(tmp._mtx);
  _evt = tmp._evt;
  _enqueue_for_deletion = std::move(tmp._enqueue_for_deletion);
  _init_send_to = std::move(tmp._init_send_to);
  _fin_send_to = std::move(tmp._fin_send_to);
  tmp._evt = nullptr;
  return *this;
}

epoll_send_to::epoll_send_to(const reactor_io_descriptor_ptr &io_desc,
                             const enqueue_for_completion_fn &enqueue_for_completion,
                             const enqueue_for_deletion_fn &enqueue_for_deletion,
                             const init_send_to_fn &init_send_to,
                             const fin_send_to_fn &fin_send_to) noexcept
    : _mtx(std::make_shared<std::mutex>()),
      _evt(event_registrar::take()),
      _enqueue_for_deletion(enqueue_for_deletion),
      _init_send_to(init_send_to),
      _fin_send_to(fin_send_to) {
  RUNTIME_ASSERT(io_desc);
  RUNTIME_ASSERT(_init_send_to);
  RUNTIME_ASSERT(_fin_send_to);
  _evt->descriptor = io_desc;
  _evt->descriptor->write_event = _evt;
  _evt->enqueue_for_completion = enqueue_for_completion;
}

epoll_send_to::~epoll_send_to() noexcept {
  // Check whether it is not default constructed
  if (_evt) {
    auto final_evt = event_registrar::take();
    final_evt->enqueue_for_completion = _evt->enqueue_for_completion;
    final_evt->complete = [evt = _evt, final_evt]() {
      event_registrar::give(evt);
      event_registrar::give(final_evt);
    };
    _enqueue_for_deletion(final_evt);
  }
}

reactor_event *epoll_send_to::evt() noexcept { return _evt; }

namespace {

void for_completion(reactor_event *evt, error_code e, const epoll_send_to::send_toed_fn &cb,
                    int bytes_recv) noexcept {
  evt->complete = [evt, e, cb, bytes_recv]() {
    // `complete` might be called in the completion thread after we are out of scope
    std::lock_guard lock(evt->in_scope_mtx);
    if (evt->in_scope) {
      cb(e, bytes_recv);
    }
  };
  evt->enqueue_for_completion(evt);
}

error_code do_react(reactor_event *evt, const epoll_send_to::fin_send_to_fn &fin_send_to,
                    uint8_t *buffer, int size, const ip_endpoint &peer_ep, int &bytes) noexcept {
  // `react` might be called in the reactor thread after we are out of scope,
  std::lock_guard lock(evt->in_scope_mtx);
  if (evt->in_scope) {
    return fin_send_to(evt->descriptor->fd, buffer, size, peer_ep, bytes);
  }
  // If we are out of scope, we just return ec::OK since we won't run the completion handler
  // anyway
  return ec::OK;
}

}  // namespace

void epoll_send_to::send_to(uint8_t *buffer, int size, const ip_endpoint &peer_ep,
                            const send_toed_fn &cb) noexcept {
  RUNTIME_ASSERT(_evt);
  RUNTIME_ASSERT(cb);
  // We lock the instance because there is a chance that this function is called
  // from the completion thread right after we call _init_send_to.
  std::lock_guard lock(*_mtx);
  // `react` might be called in the reactor thread right after calling _init_send_to,
  // so we set it before calling _init_send_to
  _evt->react = [evt = _evt, fin_send_to = _fin_send_to, buffer, size, &peer_ep, cb](error_code e) {
    if (e != ec::OK) {
      for_completion(evt, e, cb, 0);
    } else {
      int bytes = 0;
      const auto ec = do_react(evt, fin_send_to, buffer, size, peer_ep, bytes);
      for_completion(evt, ec, cb, bytes);
    }
  };
  const auto ec = _init_send_to(_evt);
  if (ec != ec::OK) {
    for_completion(_evt, ec, cb, 0);
  }
}

}  // namespace baba::os