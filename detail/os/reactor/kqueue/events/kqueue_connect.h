/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/reactor/kqueue/kqueue_reactor_event.h"
#include "os/socket/ip_endpoint.h"

namespace baba::os {

class kqueue_connect final {
 public:
  using connect_fn = std::function<error_code(io_handle fd, const ip_endpoint &peer_ep)>;

  using connect_finish_fn = std::function<void(error_code)>;

  kqueue_connect(const lifetime &scope, const std::shared_ptr<reactor_io_descriptor> &io_desc,
                 const enqueue_for_initiation_fn &enqueue_for_initiation,
                 const register_to_reactor_fn &register_to_reactor,
                 const enqueue_for_completion_fn &enqueue_for_completion,
                 const connect_fn &do_connect) noexcept;
  ~kqueue_connect() noexcept;
  void connect(const ip_endpoint &peer_ep, const connect_finish_fn &cb) noexcept;

 private:
  connect_fn _do_connect;
  reactor_event *_evt;
};

}  // namespace baba::os