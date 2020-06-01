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
struct reactor_io_descriptor;

/**
 * Function that handles the completion of an async event
 **/
using complete_fn = std::function<void()>;

/**
 * Higher order function that returns a new function to be executed in the next level
 * Parameters:
 * - kqueue doesn't have error during this time as opposed to epoll
 **/
using react_fn = std::function<void()>;

/**
 * Higher order function that returns a new function to be executed in the next level
 * Parameters:
 * - io_handle - file descriptor of the kqueue object where we initiate the async event to
 **/
using initiate_fn = std::function<void(io_handle)>;

using enqueue_for_initiation_fn = std::function<void(reactor_event *)>;
using enqueue_for_completion_fn = std::function<void(reactor_event *)>;

/**
 * Register/Unregister to/from reactor for async events
 * Parameters:
 * - io_handle - file descriptor of the kqueue object where we initiate the async event to
 * - reactor_event* - event object used as kqueue udata
 * - int - kqueue event filter
 * - unsigned int - kqueue fflags
 * - int64_t - kqueue data
 **/
using register_to_reactor_fn =
    std::function<error_code(io_handle, reactor_event *, int, unsigned int, int64_t)>;

/**
 * IO object descriptor. A descriptor can listen to either read or write events
 **/
struct reactor_io_descriptor final {
  /**
   * File descriptor of the io object (e.g. socket)
   **/
  io_handle fd = INVALID_HANDLE_VALUE;

  /**
   * Clean up the file descriptor
   **/
  ~reactor_io_descriptor() noexcept;
};

/**
 * Corresponds to a single event e.g. read/write of an io descriptor. An event maintains a
 * lifetime of the scope owning it so that completion handler is only executed if it is
 * still in scope.
 *
 * Below are the following stages that this event transitions to:
 * - initiate -> react
 * - react -> complete
 * - complete
 *
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
   * Corresponds the file descriptor that is registered to the kqueue
   **/
  std::shared_ptr<reactor_io_descriptor> descriptor;

  /**
   * Initiates the event e.g. read/write etc
   *
   * Note:
   * This is executed in the reactor thread
   **/
  initiate_fn initiate;

  /**
   * Reaction to the initiated event
   *
   * Note:
   * This is executed in the reactor thread
   **/
  react_fn react;

  /**
   * Routine executed when this event is completed
   *
   * Note:
   * This is executed in the proactor thread
   **/
  complete_fn complete;

  /**
   * Only execut if this is not expired yet
   **/
  weak_lifetime scope;

  /**
   * Id for the event registrar to use for tracking this event object
   **/
  uint64_t id;

  /**
   * The different levels of the conveyor belt
   **/
  enqueue_for_initiation_fn enqueue_for_initiation;
  register_to_reactor_fn register_to_reactor;
  enqueue_for_completion_fn enqueue_for_completion;
};

}  // namespace baba