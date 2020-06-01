/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/common/event_registrar.h"

#include "baba/semantics.h"
#include "os/reactor/epoll/epoll_reactor_event.h"

namespace baba {

reactor_event *event_registrar::take() noexcept {
  std::lock_guard lock(_mutex);
  auto &e = _register[_id_generator];
  e.id = _id_generator++;
  return &e;
}

void event_registrar::give(reactor_event *evt) noexcept {
  std::lock_guard lock(_mutex);
  RUNTIME_ASSERT(evt);
  _register.erase(evt->id);
}

void event_registrar::close() noexcept {
  // Should not lock as when we reset `react` or `complete`, they may take new events
  // for deletion and that will cause a deadlock

  // Must reset `react` and `complete` first to give stuck objects through shared_from_this()
  // to get reset and enqueue their deletion events
  for (auto &[i, evt] : _register) {
    evt.react = nullptr;
    evt.complete = nullptr;
    (void)i;
  }

  // After resetting of `react` and `complete`, we are now sure that all objects has enqueued
  // their deletion events and we just have to clear them now
  _register.clear();
}

}  // namespace baba