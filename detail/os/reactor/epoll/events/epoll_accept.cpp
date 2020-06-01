/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/events/epoll_accept.h"

#include <mutex>

namespace baba::os {

epoll_accept::epoll_accept(epoll_accept &&tmp) noexcept
    : _registrar(tmp._registrar),
      _evt(tmp._evt),
      _enqueue_for_deletion(std::move(tmp._enqueue_for_deletion)),
      _init_accept(std::move(tmp._init_accept)),
      _fin_accept(std::move(tmp._fin_accept)) {
  tmp._registrar = nullptr;
  tmp._evt = nullptr;
}

epoll_accept &epoll_accept::operator=(epoll_accept &&tmp) noexcept {
  _registrar = tmp._registrar;
  _evt = tmp._evt;
  _enqueue_for_deletion = std::move(tmp._enqueue_for_deletion);
  _init_accept = std::move(tmp._init_accept);
  _fin_accept = std::move(tmp._fin_accept);
  tmp._registrar = nullptr;
  tmp._evt = nullptr;
  return *this;
}

epoll_accept::epoll_accept(const reactor_io_descriptor_ptr &io_desc, completion_strand_impl *strand,
                           event_registrar *registrar,
                           const enqueue_for_deletion_fn &enqueue_for_deletion,
                           const init_accept_fn &init_accept,
                           const fin_accept_fn &fin_accept) noexcept
    : _registrar(registrar),
      _evt(_registrar->take()),
      _enqueue_for_deletion(enqueue_for_deletion),
      _init_accept(init_accept),
      _fin_accept(fin_accept) {
  _evt->descriptor = io_desc;
  _evt->descriptor->read_event = _evt;
  _evt->strand = strand;
}

void epoll_accept::dispose() noexcept {
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
                    const epoll_accept::accepted_fn &cb) noexcept {
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

error_code do_react(reactor_event *evt, const epoll_accept::fin_accept_fn &fin_accept,
                    const ip_endpoint &acceptor_ep, io_handle &peer_fd,
                    ip_endpoint &peer_ep) noexcept {
  // Do not process `react` if we are out of scope
  std::lock_guard lock(evt->descriptor->in_scope_mtx);
  if (evt->descriptor->in_scope) {
    return fin_accept(evt->descriptor->fd, acceptor_ep, peer_fd, peer_ep);
  }
  // If we are out of scope, we just return ec::OK since inside `complete` won't be run anymore
  return ec::OK;
}

}  // namespace

void epoll_accept::accept(const ip_endpoint &acceptor_ep, io_handle &peer_fd, ip_endpoint &peer_ep,
                          const accepted_fn &cb) noexcept {
  RUNTIME_ASSERT(_evt);
  RUNTIME_ASSERT(cb);
  // `react` might be called in the reactor thread right after calling _init_accept,
  // so we set it before calling _init_accept
  _evt->react = [evt = _evt, fin_accept = _fin_accept, &acceptor_ep, &peer_fd, &peer_ep,
                 cb](error_code e) {
    if (e != ec::OK) {
      for_completion(evt, e, cb);
    } else {
      const auto ec = do_react(evt, fin_accept, acceptor_ep, peer_fd, peer_ep);
      for_completion(evt, ec, cb);
    }
  };
  const auto ec = _init_accept(_evt);
  if (ec != ec::OK) {
    for_completion(_evt, ec, cb);
  }
}

reactor_event *epoll_accept::evt() noexcept { return _evt; }

}  // namespace baba::os