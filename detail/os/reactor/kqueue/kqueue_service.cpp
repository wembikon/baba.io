/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/kqueue/kqueue_service.h"

#include "os/impl/kqueue_impl.h"

namespace baba::os {

kqueue_service::kqueue_service() noexcept {
  _enqueue_for_initiation = [this](reactor_event* evt) {
    _initiation_queue.enqueue(evt);
    _reactor_interrupter.interrupt();
  };

  _register_to_reactor = [](io_handle kqueue_fd, reactor_event* evt, int event_filter,
                            unsigned int fflags, int64_t data) {
    return kqueue_impl::arm(kqueue_fd, evt->descriptor->fd, event_filter, fflags, data, evt);
  };

  _enqueue_for_completion = [this](reactor_event* evt) { _completion_queue.enqueue(evt); };

  _reactor_interrupter.init(_reactor.fd());
  _reactor.set_interrupter_fd(_reactor_interrupter.read_fd());
}

const enqueue_for_initiation_fn& kqueue_service::initiation_queue() const noexcept {
  return _enqueue_for_initiation;
}

const register_to_reactor_fn& kqueue_service::reactor_reg() const noexcept {
  return _register_to_reactor;
}

const enqueue_for_completion_fn& kqueue_service::completion_queue() const noexcept {
  return _enqueue_for_completion;
}

void kqueue_service::run() noexcept {
  _reactor_thread = std::thread([this]() { reactor_process(); });
  _proactor_thread = std::thread([this]() { proactor_process(); });
  _reactor_thread.join();
  _proactor_thread.join();
}

void kqueue_service::stop() noexcept {
  _run.store(false);
  _reactor_interrupter.interrupt();
  _completion_queue.stop();
}

void kqueue_service::reactor_process() noexcept {
  while (_run.load()) {
    process_initation_queue();
    _reactor.wait();
  }
}

void kqueue_service::proactor_process() noexcept {
  while (_run.load()) {
    _completion_queue.wait();
    process_completion_queue();
  }
}

void kqueue_service::process_initation_queue() noexcept {
  while (_initiation_queue.size() > 0) {
    _reactor.initiate(_initiation_queue.dequeue());
  }
}

void kqueue_service::process_completion_queue() noexcept {
  while (_completion_queue.size() > 0) {
    const auto evt = _completion_queue.dequeue();
    evt->complete();
  }
}

}  // namespace baba::os