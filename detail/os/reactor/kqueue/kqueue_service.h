/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "library/blocking_queue.h"
#include "library/safe_queue.h"
#include "os/reactor/kqueue/kqueue_interrupter.h"
#include "os/reactor/kqueue/kqueue_reactor.h"
#include "os/reactor/kqueue/kqueue_reactor_event.h"

#include <atomic>
#include <memory>
#include <thread>

namespace baba::os {

class kqueue_service final {
 public:
  NOT_COPYABLE(kqueue_service)
  NOT_MOVEABLE(kqueue_service)

  kqueue_service() noexcept;
  const enqueue_for_initiation_fn& initiation_queue() const noexcept;
  const register_to_reactor_fn& reactor_reg() const noexcept;
  const enqueue_for_completion_fn& completion_queue() const noexcept;
  void run() noexcept;
  void stop() noexcept;

 private:
  void reactor_process() noexcept;
  void proactor_process() noexcept;
  void process_initation_queue() noexcept;
  void process_completion_queue() noexcept;

  enqueue_for_initiation_fn _enqueue_for_initiation;
  register_to_reactor_fn _register_to_reactor;
  enqueue_for_completion_fn _enqueue_for_completion;

  safe_queue<reactor_event*> _initiation_queue;
  kqueue_reactor _reactor;
  blocking_queue<reactor_event*> _completion_queue;

  std::thread _reactor_thread;
  std::thread _proactor_thread;

  // The reason why this is defined here and not in the reactor is that they
  // have a strict dependency. This interrupter must be destroyed first before
  // destroying the reactor object in order for proper clean up. Thus defining
  // the reactor prior this is essential.
  kqueue_interrupter _reactor_interrupter;

  std::atomic<bool> _run = true;
};

}  // namespace baba::os