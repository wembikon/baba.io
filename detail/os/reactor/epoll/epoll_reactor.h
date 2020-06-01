/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/io_handle.h"
#include "baba/semantics.h"
#include "os/reactor/epoll/epoll_reactor_event.h"

#include <atomic>
#include <deque>
#include <functional>

#define MAX_EPOLL_EVENTS 100

namespace baba::os {

/**
 * This enables multi thread handling of reactor ready ios while ensuring proper delete
 * sequence of events by having single thread waiting for the reactor but multithread
 * processing the reactor read io or the deletion event inside a reactor task
 **/
using reactor_task_fn = std::function<void()>;
using enqueue_reactor_task_fn = std::function<void(const reactor_task_fn &)>;

class epoll_reactor final {
 public:
  NOT_COPYABLE(epoll_reactor)
  NOT_MOVEABLE(epoll_reactor)

  epoll_reactor(const enqueue_reactor_task_fn &enqueue_reactor_task) noexcept;

  /**
   * Wait for events to be available
   *
   * Returns:
   * - a reactor error. Might not be recoverable
   **/
  error_code wait() noexcept;

  /**
   * Stop the poll
   **/
  void stop() noexcept;

  /**
   * Closing the handle will also interrupt the blocking wait
   **/
  void close() noexcept;

  /**
   * Needed by interrupter
   **/
  io_handle fd() const noexcept;

  /**
   * Sets the reactor fd to interrupt
   **/
  void set_interrupter_fd(io_handle fd) noexcept;

 private:
  std::atomic<bool> _run = true;
  io_handle _fd;
  io_handle _interrupter_fd;
  struct epoll_event _event_list[MAX_EPOLL_EVENTS];
  enqueue_reactor_task_fn _enqueue_reactor_task;
};

}  // namespace baba::os