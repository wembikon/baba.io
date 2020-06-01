/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "os/reactor/epoll/epoll_reactor_event.h"
#include "os/socket/ip_endpoint.h"

#include <mutex>

namespace baba::os {

class epoll_recv_from final {
 public:
  NOT_COPYABLE(epoll_recv_from)

  using recv_fromed_fn = std::function<void(error_code, int)>;
  using init_recv_from_fn = std::function<error_code(reactor_event *)>;
  using fin_recv_from_fn =
      std::function<error_code(io_handle fd, const ip_endpoint &receiver_ep, uint8_t *buffer,
                               int size, ip_endpoint &peer_ep, int &bytes_recv)>;

  epoll_recv_from(epoll_recv_from &&tmp) noexcept;
  epoll_recv_from &operator=(epoll_recv_from &&tmp) noexcept;

  epoll_recv_from() = default;
  epoll_recv_from(const reactor_io_descriptor_ptr &io_desc,
                  const enqueue_for_completion_fn &enqueue_for_completion,
                  const enqueue_for_deletion_fn &enqueue_for_deletion,
                  const init_recv_from_fn &init_recv_from,
                  const fin_recv_from_fn &fin_recv_from) noexcept;
  ~epoll_recv_from() noexcept;
  reactor_event *evt() noexcept;
  void recv_from(const ip_endpoint &receiver_ep, uint8_t *buffer, int size, ip_endpoint &peer_ep,
                 const recv_fromed_fn &cb) noexcept;

 private:
  std::shared_ptr<std::mutex> _mtx;
  reactor_event *_evt = nullptr;
  enqueue_for_deletion_fn _enqueue_for_deletion;
  init_recv_from_fn _init_recv_from;
  fin_recv_from_fn _fin_recv_from;
};

}  // namespace baba::os