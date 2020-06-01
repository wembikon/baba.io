/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/events/epoll_send.h"

#include <mutex>

namespace baba::os {

epoll_send::epoll_send(epoll_send &&tmp) noexcept
    : _registrar(tmp._registrar),
      _evt(tmp._evt),
      _enqueue_for_deletion(std::move(tmp._enqueue_for_deletion)),
      _init_send(std::move(tmp._init_send)),
      _fin_send(std::move(tmp._fin_send)) {
  tmp._registrar = nullptr;
  tmp._evt = nullptr;
}

epoll_send &epoll_send::operator=(epoll_send &&tmp) noexcept {
  _registrar = tmp._registrar;
  _evt = tmp._evt;
  _enqueue_for_deletion = std::move(tmp._enqueue_for_deletion);
  _init_send = std::move(tmp._init_send);
  _fin_send = std::move(tmp._fin_send);
  tmp._registrar = nullptr;
  tmp._evt = nullptr;
  return *this;
}

epoll_send::epoll_send(const reactor_io_descriptor_ptr &io_desc, completion_strand_impl *strand,
                       event_registrar *registrar,
                       const enqueue_for_deletion_fn &enqueue_for_deletion,
                       const init_send_fn &init_send, const fin_send_fn &fin_send) noexcept
    : _registrar(registrar),
      _evt(_registrar->take()),
      _enqueue_for_deletion(enqueue_for_deletion),
      _init_send(init_send),
      _fin_send(fin_send) {
  _evt->descriptor = io_desc;
  _evt->descriptor->write_event = _evt;
  _evt->strand = strand;
}

void epoll_send::dispose() noexcept {
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

void for_completion(reactor_event *evt, error_code e, const epoll_send::sended_fn &cb,
                    int bytes_sent) noexcept {
  evt->complete = [evt, e, cb, bytes_sent]() {
    // `complete` handler shall be released after this scope to release captured lifetimes
    const auto holder = std::move(evt->complete);
    // Do not process `complete` if we are out of scope
    std::lock_guard lock(evt->descriptor->in_scope_mtx);
    evt->has_active_event = false;
    if (evt->descriptor->in_scope) {
      cb(e, bytes_sent);
    }
  };
  evt->react = nullptr;  // Release captured lifetime
  evt->strand->enqueue(evt);
}

error_code do_react(reactor_event *evt, const epoll_send::fin_send_fn &fin_send,
                    const uint8_t *buffer, int size, int &bytes) noexcept {
  // Do not process `react` if we are out of scope
  std::lock_guard lock(evt->descriptor->in_scope_mtx);
  if (evt->descriptor->in_scope) {
    return fin_send(evt->descriptor->fd, buffer, size, bytes);
  }
  // If we are out of scope, we just return ec::OK since inside `complete` won't be run anymore
  return ec::OK;
}

}  // namespace

void epoll_send::send(const uint8_t *buffer, int size, const sended_fn &cb) noexcept {
  RUNTIME_ASSERT(_evt);
  RUNTIME_ASSERT(cb);
  // `react` might be called in the reactor thread right after calling _init_send,
  // so we set it before calling _init_send
  _evt->react = [evt = _evt, fin_send = _fin_send, buffer, size, cb](error_code e) {
    if (e != ec::OK) {
      for_completion(evt, e, cb, 0);
    } else {
      int bytes = 0;
      const auto ec = do_react(evt, fin_send, buffer, size, bytes);
      for_completion(evt, ec, cb, bytes);
    }
  };
  const auto ec = _init_send(_evt);
  if (ec != ec::OK) {
    for_completion(_evt, ec, cb, 0);
  }
}

reactor_event *epoll_send::evt() noexcept { return _evt; }

}  // namespace baba::os