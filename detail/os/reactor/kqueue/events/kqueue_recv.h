/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/reactor/kqueue/kqueue_reactor_event.h"

namespace baba::os {

class kqueue_recv final {
 public:
  using recv_fn =
      std::function<error_code(io_handle fd, uint8_t *buffer, int size, int &bytes_recv)>;

  using receive_finish_fn = std::function<void(error_code, int)>;

  kqueue_recv(const lifetime &scope, const std::shared_ptr<reactor_io_descriptor> &io_desc,
              const enqueue_for_initiation_fn &enqueue_for_initiation,
              const register_to_reactor_fn &register_to_reactor,
              const enqueue_for_completion_fn &enqueue_for_completion,
              const recv_fn &do_recv) noexcept;
  ~kqueue_recv() noexcept;
  void recv(uint8_t *buffer, int size, const receive_finish_fn &cb) noexcept;
  void recv() noexcept;

 private:
  recv_fn _do_recv;
  reactor_event *_evt;
};

}  // namespace baba::os