/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/reactor/kqueue/kqueue_reactor_event.h"
#include "os/socket/ip_endpoint.h"

namespace baba::os {

class kqueue_recv_from final {
 public:
  using recv_from_fn =
      std::function<error_code(io_handle fd, const ip_endpoint &receiver_ep, uint8_t *buffer,
                               int size, ip_endpoint &peer_ep, int &bytes_recv)>;

  using receive_finish_fn = std::function<void(error_code, int)>;

  kqueue_recv_from(const lifetime &scope, const std::shared_ptr<reactor_io_descriptor> &io_desc,
                   const enqueue_for_initiation_fn &enqueue_for_initiation,
                   const register_to_reactor_fn &register_to_reactor,
                   const enqueue_for_completion_fn &enqueue_for_completion,
                   const recv_from_fn &do_recv_from) noexcept;
  ~kqueue_recv_from() noexcept;
  void recv_from(const ip_endpoint &receiver_ep, uint8_t *buffer, int size, ip_endpoint &peer_ep,
                 const receive_finish_fn &cb) noexcept;
  void recv_from() noexcept;

 private:
  recv_from_fn _do_recv_from;
  reactor_event *_evt;
};

}  // namespace baba::os