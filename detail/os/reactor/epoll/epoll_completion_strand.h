/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "library/blocking_queue.h"
#include "os/reactor/epoll/epoll_service.h"

#include <functional>

namespace baba::os {

/**
 * The reason why we separate the impl (completion_strand_impl) and the user interface
 * (completion_strand) is because of lifetime issues. completion_strand_impl are owned
 * by epoll_service in order to have proper shutdown sequence where the completion_strand
 * must live past the registrar because registrar close will require completion_strand
 * to still exist.
 **/

class completion_strand_impl final {
 public:
  NOT_COPYABLE(completion_strand_impl)
  NOT_MOVEABLE(completion_strand_impl)

  using task_fn = std::function<void()>;

  completion_strand_impl(epoll_service *service) noexcept;
  epoll_service &service() noexcept;
  void enqueue(reactor_event *evt) noexcept;
  void run() noexcept;
  void stop() noexcept;
  void clear() noexcept;
  void post(task_fn &&task) noexcept;

#if defined(BUILD_UNIT_TESTS)
  std::function<void(reactor_event *)> on_enqueue;
#endif

 private:
  void process_completion_queue() noexcept;

  std::atomic<bool> _run = true;
  epoll_service *_service = nullptr;
  blocking_queue<task_fn> _completion_queue;
};

/**
 * This user interface is intended for idiomatic usage of a completion_strand
 **/

class completion_strand final {
 public:
  NOT_COPYABLE(completion_strand)
  NOT_MOVEABLE(completion_strand)

  inline completion_strand(epoll_service &service) noexcept { _impl = service.create_strand(); }
  inline epoll_service &service() noexcept { return _impl->service(); }
  inline completion_strand_impl *impl() noexcept { return _impl; }
  inline void post(completion_strand_impl::task_fn &&task) noexcept {
    _impl->post(std::move(task));
  }

 private:
  completion_strand_impl *_impl;
};

}  // namespace baba::os