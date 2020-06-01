/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/reactor/kqueue/kqueue_reactor_event.h"
#include "os/socket/ip_endpoint.h"

namespace baba::os {

class kqueue_send_to final {
 public:
  using send_to_fn = std::function<error_code(io_handle fd, uint8_t *buffer, int size,
                                              const ip_endpoint &peer_ep, int &bytes_sent)>;

  using send_finish_fn = std::function<void(error_code, int)>;

  kqueue_send_to(const lifetime &scope, const std::shared_ptr<reactor_io_descriptor> &io_desc,
                 const enqueue_for_initiation_fn &enqueue_for_initiation,
                 const register_to_reactor_fn &register_to_reactor,
                 const enqueue_for_completion_fn &enqueue_for_completion,
                 const send_to_fn &do_send_to) noexcept;
  ~kqueue_send_to() noexcept;
  void send_to(uint8_t *buffer, int size, const ip_endpoint &peer_ep,
               const send_finish_fn &cb) noexcept;
  void send_to() noexcept;

 private:
  send_to_fn _do_send_to;
  reactor_event *_evt;
};

}  // namespace baba::os