/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/iocp/iocp_service.h"

#include "os/impl/iocp_impl.h"

namespace baba::os {

iocp_service::iocp_service() noexcept {
  _enqueue_for_completion = [this](reactor_event* evt) { _completion_queue.enqueue(evt); };
  _enqueue_for_deletion = [this](reactor_event* evt) { _deletion_queue.enqueue(evt); };
}

const iocp_reactor& iocp_service::reactor() const { return _reactor; }

const enqueue_for_completion_fn& iocp_service::completion_queue() const noexcept {
  return _enqueue_for_completion;
}

const enqueue_for_deletion_fn& iocp_service::deletion_queue() const noexcept {
  return _enqueue_for_deletion;
}

void iocp_service::run() noexcept {
  _reactor_thread = std::thread([this]() { reactor_process(); });
  _proactor_thread = std::thread([this]() { proactor_process(); });
  _reactor_thread.join();
  _proactor_thread.join();
}

void iocp_service::stop() noexcept {
  _run.store(false);
  _completion_queue.stop();
  _reactor.close();
}

void iocp_service::reactor_process() noexcept {
  while (_run.load()) {
    process_deletion_queue();
    // Maybe check the error code?
    _reactor.wait();
  }
}

void iocp_service::proactor_process() noexcept {
  while (_run.load()) {
    _completion_queue.wait();
    process_completion_queue();
  }
}

void iocp_service::process_completion_queue() noexcept {
  while (_completion_queue.size() > 0) {
    const auto evt = _completion_queue.dequeue();
    evt->complete();
  }
}

void iocp_service::process_deletion_queue() noexcept {
  while (_deletion_queue.size() > 0) {
    const auto evt = _deletion_queue.dequeue();
    // We cannot delete in the reactor thread since we are not sure if there are
    // still pending event in the completion queue. We instead put this into the
    // completion queue so that it will be executed after all events in the completion
    // queue has been executed.
    evt->enqueue_for_completion(evt);
  }
}

}  // namespace baba::os