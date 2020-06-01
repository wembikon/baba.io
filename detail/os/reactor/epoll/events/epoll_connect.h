/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "os/common/event_registrar.h"
#include "os/reactor/epoll/epoll_completion_strand.h"
#include "os/reactor/epoll/epoll_reactor_event.h"
#include "os/socket/ip_endpoint.h"

namespace baba::os {

class epoll_connect final {
 public:
  NOT_COPYABLE(epoll_connect)

  using connected_fn = std::function<void(error_code)>;
  using init_connect_fn = std::function<error_code(reactor_event *)>;
  using fin_connect_fn = std::function<error_code(io_handle fd, const ip_endpoint &peer_ep)>;

  epoll_connect(epoll_connect &&tmp) noexcept;
  epoll_connect &operator=(epoll_connect &&tmp) noexcept;

  epoll_connect() = default;
  epoll_connect(const reactor_io_descriptor_ptr &io_desc, completion_strand_impl *strand,
                event_registrar *registrar, const enqueue_for_deletion_fn &enqueue_for_deletion,
                const init_connect_fn &init_connect, const fin_connect_fn &fin_connect) noexcept;
  void dispose() noexcept;

  void connect(const ip_endpoint &peer_ep, const connected_fn &cb) noexcept;

  reactor_event *evt() noexcept;

 private:
  event_registrar *_registrar = nullptr;
  reactor_event *_evt = nullptr;
  enqueue_for_deletion_fn _enqueue_for_deletion;
  init_connect_fn _init_connect;
  fin_connect_fn _fin_connect;
};

}  // namespace baba::os