/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/kqueue/events/kqueue_connect.h"

#include "baba/semantics.h"
#include "os/common/event_registrar.h"

namespace baba::os {

kqueue_connect::kqueue_connect(const lifetime &scope,
                               const std::shared_ptr<reactor_io_descriptor> &io_desc,
                               const enqueue_for_initiation_fn &enqueue_for_initiation,
                               const register_to_reactor_fn &register_to_reactor,
                               const enqueue_for_completion_fn &enqueue_for_completion,
                               const connect_fn &do_connect) noexcept
    : _do_connect(do_connect), _evt(event_registrar::take()) {
  RUNTIME_ASSERT(scope);
  RUNTIME_ASSERT(io_desc);
  _evt->scope = scope;
  _evt->descriptor = io_desc;
  _evt->enqueue_for_initiation = enqueue_for_initiation;
  _evt->register_to_reactor = register_to_reactor;
  _evt->enqueue_for_completion = enqueue_for_completion;
}

kqueue_connect::~kqueue_connect() noexcept {
  auto final_evt = event_registrar::take();
  final_evt->initiate = [final_evt, evt = _evt](io_handle) {
    final_evt->complete = [final_evt, evt]() {
      event_registrar::give(evt);
      event_registrar::give(final_evt);
    };
    evt->enqueue_for_completion(final_evt);
  };
  _evt->enqueue_for_initiation(final_evt);
}

void kqueue_connect::connect(const ip_endpoint &peer_ep, const connect_finish_fn &cb) noexcept {
  _evt->initiate = [evt = _evt, do_connect = _do_connect, &peer_ep, cb](io_handle kqueue_fd) {
    auto ec = evt->register_to_reactor(kqueue_fd, evt, EVFILT_WRITE, 0, 0);
    if (ec == ec::OK) {
      // The socket will be writable (receive EVFILT_WRITE) once it is connected
      ec = do_connect(evt->descriptor->fd, peer_ep);
    }
    if (ec != ec::OK) {
      evt->complete = [ec, cb]() { cb(ec); };
      evt->enqueue_for_completion(evt);
    }
  };
  _evt->react = [evt = _evt, cb]() {
    evt->complete = [cb]() { cb(ec::OK); };
    evt->enqueue_for_completion(evt);
  };
  _evt->enqueue_for_initiation(_evt);
}

}  // namespace baba::os