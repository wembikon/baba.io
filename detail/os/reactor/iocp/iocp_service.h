/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "library/blocking_queue.h"
#include "library/safe_queue.h"
#include "os/reactor/iocp/iocp_reactor.h"
#include "os/reactor/iocp/iocp_reactor_event.h"

#include <atomic>
#include <memory>
#include <thread>

namespace baba::os {

class iocp_service final {
 public:
  NOT_COPYABLE(iocp_service)
  NOT_MOVEABLE(iocp_service)

  iocp_service() noexcept;
  const iocp_reactor& reactor() const;
  const enqueue_for_completion_fn& completion_queue() const noexcept;
  const enqueue_for_deletion_fn& deletion_queue() const noexcept;
  void run() noexcept;
  void stop() noexcept;

 private:
  void reactor_process() noexcept;
  void proactor_process() noexcept;
  void process_completion_queue() noexcept;
  void process_deletion_queue() noexcept;

  enqueue_for_completion_fn _enqueue_for_completion;
  enqueue_for_deletion_fn _enqueue_for_deletion;

  iocp_reactor _reactor;
  blocking_queue<reactor_event*> _completion_queue;
  safe_queue<reactor_event*> _deletion_queue;

  std::thread _reactor_thread;
  std::thread _proactor_thread;

  std::atomic<bool> _run = true;
};

}  // namespace baba::os