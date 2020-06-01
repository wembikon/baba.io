/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/iocp/events/iocp_timer.h"

#include "baba/semantics.h"
#include "os/common/event_registrar.h"

namespace baba::os {

iocp_timer::iocp_timer(const lifetime &scope, const reactor_io_descriptor_ptr &io_desc,
                       const enqueue_for_completion_fn &enqueue_for_completion,
                       const enqueue_for_deletion_fn &enqueue_for_deletion,
                       const open_timer_fn &open_timer, const close_timer_fn &close_timer,
                       const init_timer_fn &init_timer) noexcept
    : _evt(event_registrar::take()),
      _enqueue_for_deletion(enqueue_for_deletion),
      _init_timer(init_timer),
      _close_timer(close_timer) {
  RUNTIME_ASSERT(scope);
  RUNTIME_ASSERT(io_desc);
  _evt->scope = scope;
  _evt->descriptor = io_desc;
  _evt->enqueue_for_completion = enqueue_for_completion;
  open_timer(_evt);
}

iocp_timer::~iocp_timer() noexcept {
  // It is essential to close the timer first so it gets removed from the reactor before we
  // enqueue the deletion routine. This will ensure that when the reactor loops into the
  // deletion queue, the file descriptor is not in the reactor anymore (i.e. also the event)
  // object, thus making sure that the event object is safe to delete.
  _close_timer(_evt);
  auto final_evt = event_registrar::take();
  final_evt->complete = [evt = _evt, final_evt]() {
    event_registrar::give(evt);
    event_registrar::give(final_evt);
  };
  _enqueue_for_deletion(final_evt);
}

void iocp_timer::timeout(uint32_t timeout_ms, const expired_fn &cb) noexcept {
  RUNTIME_ASSERT(cb);
  // We lock the instance because there is a chance that this function is called
  // from the completion thread right after we call _init_timer.
  std::lock_guard lock(_mtx);
  // `react` might be called right after calling _init_timer, so we set it first
  _evt->react = [evt = _evt, cb]() {
    evt->complete = [cb = std::move(cb)]() { cb(ec::OK); };
    evt->enqueue_for_completion(evt);
  };
  if (const auto ec = _init_timer(_evt->descriptor->fd, timeout_ms); ec != ec::OK) {
    _evt->complete = [cb, ec]() { cb(ec); };
    _evt->enqueue_for_completion(_evt);
  }
}

}  // namespace baba::os