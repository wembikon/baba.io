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

class epoll_accept final {
 public:
  NOT_COPYABLE(epoll_accept)

  using accepted_fn = std::function<void(error_code)>;
  using init_accept_fn = std::function<error_code(reactor_event *)>;
  using fin_accept_fn =
      std::function<error_code(io_handle acceptor_fd, const ip_endpoint &acceptor_ep,
                               io_handle &peer_fd, ip_endpoint &peer_ep)>;

  epoll_accept(epoll_accept &&tmp) noexcept;
  epoll_accept &operator=(epoll_accept &&tmp) noexcept;

  epoll_accept() = default;
  epoll_accept(const reactor_io_descriptor_ptr &io_desc, completion_strand_impl *strand,
               event_registrar *registrar, const enqueue_for_deletion_fn &enqueue_for_deletion,
               const init_accept_fn &init_accept, const fin_accept_fn &fin_accept) noexcept;
  void dispose() noexcept;

  void accept(const ip_endpoint &acceptor_ep, io_handle &peer_fd, ip_endpoint &peer_ep,
              const accepted_fn &cb) noexcept;

  reactor_event *evt() noexcept;

 private:
  event_registrar *_registrar = nullptr;
  reactor_event *_evt = nullptr;
  enqueue_for_deletion_fn _enqueue_for_deletion;
  init_accept_fn _init_accept;
  fin_accept_fn _fin_accept;
};

}  // namespace baba::os