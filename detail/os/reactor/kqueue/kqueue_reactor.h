/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/io_handle.h"
#include "baba/semantics.h"
#include "os/reactor/kqueue/kqueue_reactor_event.h"

#include <deque>

#define MAX_KQUEUE_EVENTS 100

namespace baba::os {

class kqueue_reactor final {
 public:
  NOT_COPYABLE(kqueue_reactor)
  NOT_MOVEABLE(kqueue_reactor)

  kqueue_reactor() noexcept;
  ~kqueue_reactor() noexcept;

  /**
   * Calls the `initiate` routine of the reactor_event
   **/
  void initiate(reactor_event *evt) noexcept;

  /**
   * Wait for events to be available
   *
   * Returns:
   * - a reactor error. Might not be recoverable
   **/
  error_code wait() noexcept;

  /**
   * Needed by interrupter
   **/
  io_handle fd() noexcept;

  /**
   * For the reactor to make an exception for interrupter fd events
   **/
  void set_interrupter_fd(io_handle fd) noexcept;

 private:
  io_handle _fd;
  io_handle _interrupter_fd;
  struct kevent _event_list[MAX_KQUEUE_EVENTS];
};

}  // namespace baba::os