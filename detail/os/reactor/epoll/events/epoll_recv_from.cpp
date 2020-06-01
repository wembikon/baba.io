/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/events/epoll_recv_from.h"

#include "os/common/event_registrar.h"

namespace baba::os {

epoll_recv_from::epoll_recv_from(epoll_recv_from &&tmp) noexcept
    : _mtx(std::move(tmp._mtx)),
      _evt(tmp._evt),
      _enqueue_for_deletion(std::move(tmp._enqueue_for_deletion)),
      _init_recv_from(std::move(tmp._init_recv_from)),
      _fin_recv_from(std::move(tmp._fin_recv_from)) {
  tmp._evt = nullptr;
}

epoll_recv_from &epoll_recv_from::operator=(epoll_recv_from &&tmp) noexcept {
  _mtx = std::move(tmp._mtx);
  _evt = tmp._evt;
  _enqueue_for_deletion = std::move(tmp._enqueue_for_deletion);
  _init_recv_from = std::move(tmp._init_recv_from);
  _fin_recv_from = std::move(tmp._fin_recv_from);
  tmp._evt = nullptr;
  return *this;
}

epoll_recv_from::epoll_recv_from(const reactor_io_descriptor_ptr &io_desc,
                                 const enqueue_for_completion_fn &enqueue_for_completion,
                                 const enqueue_for_deletion_fn &enqueue_for_deletion,
                                 const init_recv_from_fn &init_recv_from,
                                 const fin_recv_from_fn &fin_recv_from) noexcept
    : _mtx(std::make_shared<std::mutex>()),
      _evt(event_registrar::take()),
      _enqueue_for_deletion(enqueue_for_deletion),
      _init_recv_from(init_recv_from),
      _fin_recv_from(fin_recv_from) {
  RUNTIME_ASSERT(io_desc);
  RUNTIME_ASSERT(_init_recv_from);
  RUNTIME_ASSERT(_fin_recv_from);
  _evt->descriptor = io_desc;
  _evt->descriptor->read_event = _evt;
  _evt->enqueue_for_completion = enqueue_for_completion;
}

epoll_recv_from::~epoll_recv_from() noexcept {
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

reactor_event *epoll_recv_from::evt() noexcept { return _evt; }

namespace {

void for_completion(reactor_event *evt, error_code e, const epoll_recv_from::recv_fromed_fn &cb,
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

error_code do_react(reactor_event *evt, const epoll_recv_from::fin_recv_from_fn &fin_recv_from,
                    const ip_endpoint &receiver_ep, uint8_t *buffer, int size, ip_endpoint &peer_ep,
                    int &bytes) noexcept {
  // `react` might be called in the reactor thread after we are out of scope,
  std::lock_guard lock(evt->in_scope_mtx);
  if (evt->in_scope) {
    return fin_recv_from(evt->descriptor->fd, receiver_ep, buffer, size, peer_ep, bytes);
  }
  // If we are out of scope, we just return ec::OK since we won't run the completion handler
  // anyway
  return ec::OK;
}

}  // namespace

void epoll_recv_from::recv_from(const ip_endpoint &receiver_ep, uint8_t *buffer, int size,
                                ip_endpoint &peer_ep, const recv_fromed_fn &cb) noexcept {
  RUNTIME_ASSERT(_evt);
  RUNTIME_ASSERT(cb);
  // We lock the instance because there is a chance that this function is called
  // from the completion thread right after we call _init_recv_from.
  std::lock_guard lock(*_mtx);
  // `react` might be called in the reactor thread right after calling _init_recv_from,
  // so we set it before calling _init_recv_from
  _evt->react = [evt = _evt, fin_recv_from = _fin_recv_from, &receiver_ep, buffer, size, &peer_ep,
                 cb](error_code e) {
    if (e != ec::OK) {
      for_completion(evt, e, cb, 0);
    } else {
      int bytes = 0;
      const auto ec = do_react(evt, fin_recv_from, receiver_ep, buffer, size, peer_ep, bytes);
      for_completion(evt, ec, cb, bytes);
    }
  };
  const auto ec = _init_recv_from(_evt);
  if (ec != ec::OK) {
    for_completion(_evt, ec, cb, 0);
  }
}

}  // namespace baba::os