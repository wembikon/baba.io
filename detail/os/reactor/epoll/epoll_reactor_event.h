/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/io_types.h"
#include "baba/lifetime.h"

#include <functional>
#include <memory>
#include <mutex>

namespace baba::os {
class completion_strand_impl;
}

namespace baba {

struct reactor_event;

using react_fn = std::function<void(error_code)>;
using complete_fn = std::function<void()>;
using enqueue_for_deletion_fn = std::function<void(reactor_event *)>;

/**
 * IO object descriptor. A descriptor can listen to either read or write events
 **/
struct reactor_io_descriptor final {
  /**
   * File descriptor of the io object (e.g. socket)
   **/
  io_handle fd = INVALID_HANDLE_VALUE;

  /**
   * Since Epoll doesn't listen for individual events, rather it listens for the whole
   * descriptor, we need to branch out the events from the descriptor
   *
   * Uses a raw pointer to not have cyclic dependency. If reactor_event is deleted,
   * it should also mean that reactor_io_descriptor is also deleted vise versa
   **/
  reactor_event *read_event;
  reactor_event *write_event;

  /**
   * When intiating an event we should save the event mask because epoll needs to be rearmed
   * when using EPOLLONESHOT
   **/
  uint32_t event_mask = 0;

  /**
   * This mutex makes sure that we only process `react` and `complete` if we are still in
   * scope. This in_scope will be set to true by the destructor of the owning object (e.g.
   * epoll_async_acceptor) which is also protected by the same mutex. In short, there are
   * 3 calls that are protected by this mutex, `react`, `complete` and the destructor of the
   * owning object.
   **/
  std::recursive_mutex in_scope_mtx;
  bool in_scope = true;

  /**
   * Set event mask
   **/
  void set_read_mask() noexcept;
  void set_write_mask() noexcept;

  /**
   * Clear event masks
   **/
  void clear_read_mask() noexcept;
  void clear_write_mask() noexcept;
};

using reactor_io_descriptor_ptr = std::shared_ptr<reactor_io_descriptor>;

/**
 * Corresponds to a single event e.g. read/write/timeout/accept/connect
 **/
struct reactor_event final {
#if defined(BUILD_UNIT_TESTS)
  std::function<void()> on_delete;
  ~reactor_event() noexcept {
    if (on_delete) {
      on_delete();
    }
  }
#endif

  /**
   * Corresponds the file descriptor that is registered to the epoll. Must be a shared_ptr
   * because individual events hold to it after the object that created the descriptor
   * is already deleted
   **/
  reactor_io_descriptor_ptr descriptor;

  /**
   * Reaction to the initiated event
   **/
  react_fn react;

  /**
   * Routine executed when this event is completed or when this event object
   * is going to be deleted.
   **/
  complete_fn complete;

  /**
   * During destruction of this event, it is possible that an active event and a final event
   * competes with each other for the completion queue (this is due to the fact that it is
   * possible that we are running multiple reactor threads which can enqueue either events
   * to the completion queue). If a final event is processed but the event that it is trying
   * to destroy still has an active event, we need to postpone that by re-enqueueing the
   * final event to the completion queue until the pending event is processed.
   **/
  bool has_active_event = false;

  /**
   * Completion strand where the completion handler is executed
   **/
  os::completion_strand_impl *strand = nullptr;

  /**
   * ID for the registrar to use to track this event object
   **/
  uint64_t id = 0;
};

}  // namespace baba