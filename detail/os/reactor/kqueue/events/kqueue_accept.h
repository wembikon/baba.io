/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/reactor/kqueue/kqueue_reactor_event.h"
#include "os/socket/ip_endpoint.h"

namespace baba::os {

class kqueue_accept final {
 public:
  using accept_fn = std::function<error_code(io_handle acceptor_fd, const ip_endpoint &acceptor_ep,
                                             io_handle &peer_fd, ip_endpoint &peer_ep)>;

  using accept_finish_fn = std::function<void(error_code)>;

  kqueue_accept(const lifetime &scope, const std::shared_ptr<reactor_io_descriptor> &io_desc,
                const enqueue_for_initiation_fn &enqueue_for_initiation,
                const register_to_reactor_fn &register_to_reactor,
                const enqueue_for_completion_fn &enqueue_for_completion,
                const accept_fn &do_accept) noexcept;
  ~kqueue_accept() noexcept;
  void accept(const ip_endpoint &acceptor_ep, io_handle &peer_fd, ip_endpoint &peer_ep,
              const accept_finish_fn &cb) noexcept;
  void accept() noexcept;

 private:
  accept_fn _do_accept;
  reactor_event *_evt;
};

}  // namespace baba::os