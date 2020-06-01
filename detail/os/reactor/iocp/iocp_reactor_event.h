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

namespace baba {

struct reactor_event;

using react_fn = std::function<void()>;
using complete_fn = std::function<void()>;
using dispose_fn = std::function<void()>;
using enqueue_for_completion_fn = std::function<void(reactor_event *)>;
using enqueue_for_deletion_fn = std::function<void(reactor_event *)>;

/**
 * IO object descriptor. A descriptor can listen to either read or write events
 **/
struct reactor_io_descriptor final {
  /**
   * Handle of the io object (e.g. timer, socket etc)
   **/
  io_handle fd = INVALID_HANDLE_VALUE;
};

using reactor_io_descriptor_ptr = std::shared_ptr<reactor_io_descriptor>;

/**
 * Corresponds to a single event e.g. read/write/timeout
 **/
struct reactor_event final : public OVERLAPPED {
#if defined(BUILD_UNIT_TESTS)
  std::function<void()> on_delete;
  ~reactor_event() noexcept {
    if (on_delete) {
      on_delete();
    }
  }
#endif

  /**
   * Handle of the io object (e.g. timer, socket etc) that cleans itself when destroyed
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
   * Only execute if this is not expired yet
   **/
  weak_lifetime scope;

  /**
   * Id for the event registrar to use for tracking this event object
   **/
  uint64_t id;

  /**
   * The completion queue
   **/
  enqueue_for_completion_fn enqueue_for_completion;
};

}  // namespace baba