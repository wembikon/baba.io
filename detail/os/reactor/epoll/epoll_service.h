/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "library/blocking_queue.h"
#include "library/lockable_queue.h"
#include "os/common/event_registrar.h"
#include "os/reactor/epoll/epoll_interrupter.h"
#include "os/reactor/epoll/epoll_reactor.h"
#include "os/reactor/epoll/epoll_reactor_event.h"

#include <atomic>
#include <memory>
#include <thread>
#include <vector>

namespace baba::os {

class completion_strand_impl;

class epoll_service final {
 public:
  NOT_COPYABLE(epoll_service)
  NOT_MOVEABLE(epoll_service)

  epoll_service(int reactor_threads = 0) noexcept;
  ~epoll_service() noexcept;
  completion_strand_impl* create_strand() noexcept;
  const epoll_reactor& reactor() const;
  event_registrar* registrar() noexcept;
  const enqueue_for_deletion_fn& deletion_queue() const noexcept;
  void run() noexcept;
  void stop() noexcept;

 private:
  void start_reactor() noexcept;
  void start_reactor_task_processor() noexcept;
  void start_completion_strands() noexcept;

  void reactor_process() noexcept;
  void reactor_task_process() noexcept;
  void proactor_process(completion_strand_impl* strand) noexcept;

  void process_reactor_queue() noexcept;
  void process_deletion_queue() noexcept;

  std::atomic<bool> _run = true;

  std::vector<std::shared_ptr<completion_strand_impl>> _completion_strands;

  epoll_reactor _reactor;
  epoll_interrupter _reactor_interrupter;

  event_registrar _registrar;

  blocking_queue<reactor_task_fn> _reactor_task_queue;

  lockable_queue<reactor_event*> _deletion_queue;
  enqueue_for_deletion_fn _enqueue_for_deletion;

  int _num_reactor_threads = 0;

  std::thread _reactor_thread;
  std::vector<std::thread> _reactor_task_threads;
  std::vector<std::thread> _completion_threads;
};

}  // namespace baba::os