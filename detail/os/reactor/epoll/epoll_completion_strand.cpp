/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/epoll_completion_strand.h"

namespace baba::os {

completion_strand_impl::completion_strand_impl(epoll_service *service) noexcept
    : _service(service) {
  RUNTIME_ASSERT(_service);
}

epoll_service &completion_strand_impl::service() noexcept { return *_service; }

void completion_strand_impl::enqueue(reactor_event *evt) noexcept {
  _completion_queue.enqueue([evt]() { evt->complete(); });
#if defined(BUILD_UNIT_TESTS)
  if (on_enqueue) {
    on_enqueue(evt);
  }
#endif
}

void completion_strand_impl::run() noexcept {
  _completion_queue.wait();
  process_completion_queue();
}

void completion_strand_impl::stop() noexcept {
  _run.store(false);
  _completion_queue.stop();
}

void completion_strand_impl::clear() noexcept { _completion_queue.clear(); }

void completion_strand_impl::post(task_fn &&task) noexcept {
  RUNTIME_ASSERT(task);
  _completion_queue.enqueue(std::move(task));
}

void completion_strand_impl::process_completion_queue() noexcept {
  for (auto task = _completion_queue.dequeue(); _run.load() && task;
       task = _completion_queue.dequeue()) {
    (*task)();
  }
}

}  // namespace baba::os