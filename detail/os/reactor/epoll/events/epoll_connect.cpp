/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/events/epoll_connect.h"

#include <mutex>

namespace baba::os {

epoll_connect::epoll_connect(epoll_connect &&tmp) noexcept
    : _registrar(tmp._registrar),
      _evt(tmp._evt),
      _enqueue_for_deletion(std::move(tmp._enqueue_for_deletion)),
      _init_connect(std::move(tmp._init_connect)),
      _fin_connect(std::move(tmp._fin_connect)) {
  tmp._registrar = nullptr;
  tmp._evt = nullptr;
}

epoll_connect &epoll_connect::operator=(epoll_connect &&tmp) noexcept {
  _registrar = tmp._registrar;
  _evt = tmp._evt;
  _enqueue_for_deletion = std::move(tmp._enqueue_for_deletion);
  _init_connect = std::move(tmp._init_connect);
  _fin_connect = std::move(tmp._fin_connect);
  tmp._registrar = nullptr;
  tmp._evt = nullptr;
  return *this;
}

epoll_connect::epoll_connect(const reactor_io_descriptor_ptr &io_desc,
                             completion_strand_impl *strand, event_registrar *registrar,
                             const enqueue_for_deletion_fn &enqueue_for_deletion,
                             const init_connect_fn &init_connect,
                             const fin_connect_fn &fin_connect) noexcept
    : _registrar(registrar),
      _evt(_registrar->take()),
      _enqueue_for_deletion(enqueue_for_deletion),
      _init_connect(init_connect),
      _fin_connect(fin_connect) {
  _evt->descriptor = io_desc;
  _evt->descriptor->write_event = _evt;
  _evt->strand = strand;
}

void epoll_connect::dispose() noexcept {
  // Check whether it is not default constructed
  if (_evt) {
    auto final_evt = _registrar->take();
    final_evt->strand = _evt->strand;
    final_evt->complete = [registrar = _registrar, evt = _evt, final_evt]() {
      // After we remove the event in the Epoll, we cannot be sure that it is enqueued to the
      // completion queue. So if a final event detects that there is still an active event behind
      // it in the queue, we shall make that active event the final event instead. This is safe
      // since all this are run on the same strand and we are sure that they are run serially.
      if (evt->has_active_event) {
        final_evt->strand->enqueue(final_evt);
      } else {
        registrar->give(evt);
        registrar->give(final_evt);
      }
    };
    _enqueue_for_deletion(final_evt);
  }
}

namespace {

void for_completion(reactor_event *evt, error_code e,
                    const epoll_connect::connected_fn &cb) noexcept {
  evt->complete = [evt, e, cb]() {
    // `complete` handler shall be released after this scope to release captured lifetimes
    const auto holder = std::move(evt->complete);
    // Do not process `complete` if we are out of scope
    std::lock_guard lock(evt->descriptor->in_scope_mtx);
    evt->has_active_event = false;
    if (evt->descriptor->in_scope) {
      cb(e);
    }
  };
  evt->react = nullptr;  // Release captured lifetime
  evt->strand->enqueue(evt);
}

// No do_react in `connect` because `fin_connect` is called right after `init_connect`
// and during `react` we just enqueue directly the event to the completion queue

}  // namespace

void epoll_connect::connect(const ip_endpoint &peer_ep, const connected_fn &cb) noexcept {
  RUNTIME_ASSERT(_evt);
  RUNTIME_ASSERT(cb);
  // The socket will be writable (receive EPOLLOUT) once it is connected, or in case of
  // error, the error will be delivery through a `react`
  const auto e = _fin_connect(_evt->descriptor->fd, peer_ep);
  // `react` might be called in the reactor thread right after calling _init_connect,
  // so we set it before calling _init_connect
  _evt->react = [evt = _evt, cb, e](error_code) {
    // Epoll will always fire an EPOLLOUT instead of EPOLLERR even _fin_connect had an
    // error, that is why we just call _fin_connect first and pass the error code to
    // the `react` function so that the correct error code is passed
    for_completion(evt, e, cb);
  };
  const auto ec = _init_connect(_evt);
  if (ec != ec::OK) {
    for_completion(_evt, ec, cb);
  }
}

reactor_event *epoll_connect::evt() noexcept { return _evt; }

}  // namespace baba::os