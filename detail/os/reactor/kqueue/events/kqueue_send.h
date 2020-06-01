/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/reactor/kqueue/kqueue_reactor_event.h"

namespace baba::os {

class kqueue_send final {
 public:
  using send_fn =
      std::function<error_code(io_handle fd, uint8_t *buffer, int size, int &bytes_sent)>;

  using send_finish_fn = std::function<void(error_code, int)>;

  kqueue_send(const lifetime &scope, const std::shared_ptr<reactor_io_descriptor> &io_desc,
              const enqueue_for_initiation_fn &enqueue_for_initiation,
              const register_to_reactor_fn &register_to_reactor,
              const enqueue_for_completion_fn &enqueue_for_completion,
              const send_fn &do_send) noexcept;
  ~kqueue_send() noexcept;
  void send(uint8_t *buffer, int size, const send_finish_fn &cb) noexcept;
  void send() noexcept;

 private:
  send_fn _do_send;
  reactor_event *_evt;
};

}  // namespace baba::os