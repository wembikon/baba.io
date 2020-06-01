/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/epoll_service.h"

#include "os/impl/epoll_impl.h"
#include "os/reactor/epoll/epoll_completion_strand.h"

namespace baba::os {

epoll_service::epoll_service(int reactor_threads) noexcept
    : _reactor([this](const reactor_task_fn& task) { _reactor_task_queue.enqueue(task); }) {
  if (reactor_threads == 0) {
    // TODO: perhaps get the number of processors in a hardware? For now defaults to 4
    _num_reactor_threads = 4;
  }
  _enqueue_for_deletion = [this](reactor_event* evt) {
    _deletion_queue.enqueue(evt);
    _reactor_interrupter.interrupt();
  };

  _reactor_interrupter.init(_reactor.fd());
  _reactor.set_interrupter_fd(_reactor_interrupter.read_fd());
}

epoll_service::~epoll_service() noexcept {
  // During this time, we are sure that there are no other threads running (reactor
  // and proactor threads are joined) so it should give us assurance that we are
  // now operating on a single thread and not thread issues should happen

  // Also clear the posted tasks since they might capture lifetimes
  for (auto& strand : _completion_strands) {
    strand->clear();
  }

  // Only close registrar when destructing the service so that users only need to
  // make sure that all their event objects live within the scope of the service
  _registrar.close();

  // Only close the interrupter after _registrar.close because it will delete event
  // objects and those will use the interrupter during dispose
  _reactor_interrupter.close();

  // This prevents event objects that exits at the end of the program from getting
  // error deleting fd from the epoll instance.
  _reactor.close();

  // Strands will be destroyed after this. Strands must exist when closing the reigstrar
  // since deletion events will still require them
}

completion_strand_impl* epoll_service::create_strand() noexcept {
  const auto& strand = _completion_strands.emplace_back(new completion_strand_impl(this));
  return strand.get();
}

const epoll_reactor& epoll_service::reactor() const { return _reactor; }

const enqueue_for_deletion_fn& epoll_service::deletion_queue() const noexcept {
  return _enqueue_for_deletion;
}

event_registrar* epoll_service::registrar() noexcept { return &_registrar; }

void epoll_service::run() noexcept {
  start_reactor();
  start_reactor_task_processor();
  start_completion_strands();
  for (auto i = begin(_reactor_task_threads); i != end(_reactor_task_threads); ++i) {
    i->join();
  }
  _reactor_thread.join();
  for (auto i = begin(_completion_threads); i != end(_completion_threads); ++i) {
    i->join();
  }
}

void epoll_service::stop() noexcept {
  _run.store(false);
  for (auto& strand : _completion_strands) {
    strand->stop();
  }
  _reactor_task_queue.stop();
  _reactor.stop();
  _reactor_interrupter.interrupt();
}

void epoll_service::start_reactor() noexcept {
  _reactor_thread = std::thread([this]() { reactor_process(); });
}

void epoll_service::start_reactor_task_processor() noexcept {
  for (auto i = 0; i != _num_reactor_threads; ++i) {
    _reactor_task_threads.emplace_back([this]() { reactor_task_process(); });
  }
}

void epoll_service::start_completion_strands() noexcept {
  RUNTIME_ASSERT(_completion_strands.size() > 0);
  for (auto i = begin(_completion_strands) + 1; i != end(_completion_strands); ++i) {
    _completion_threads.emplace_back([this, strand = (*i).get()]() { proactor_process(strand); });
  }
  // Default strand will run on this thread
  proactor_process(_completion_strands[0].get());
}

void epoll_service::reactor_process() noexcept {
  while (_run.load()) {
    // Maybe check the error code?
    _reactor.wait();
    process_deletion_queue();
  }
}

void epoll_service::reactor_task_process() noexcept {
  while (_run.load()) {
    _reactor_task_queue.wait();
    process_reactor_queue();
  }
}

void epoll_service::proactor_process(completion_strand_impl* strand) noexcept {
  while (_run.load()) {
    strand->run();
  }
}

void epoll_service::process_reactor_queue() noexcept {
  for (auto task = _reactor_task_queue.dequeue(); _run.load() && task;
       task = _reactor_task_queue.dequeue()) {
    (*task)();
  }
}

void epoll_service::process_deletion_queue() noexcept {
  // This is very important to ensure deletion events are executed after any events that
  // might be in the reactor previously. Basically if there are events currently in the
  // reactor and we destruct the object, we need to make sure that those events have
  // possibly transitioned from the reactor to the reactor ready io queue. So we execute
  // deletion events sequentially with the reactor ready io queue to ensure that we run
  // those events that ends up in the reactor ready io queue before its own deletion event.
  std::lock_guard freeze(_deletion_queue.mtx());
  while (_run.load() && _deletion_queue.size() > 0) {
    const auto evt = _deletion_queue.dequeue();
    // We cannot delete in the reactor thread since we are not sure if there are
    // still pending event in the completion queue. We instead put this into the
    // completion queue so that it will be executed after all events in the completion
    // queue has been executed.
    _reactor_task_queue.enqueue([evt]() { evt->strand->enqueue(evt); });
  }
}

}  // namespace baba::os