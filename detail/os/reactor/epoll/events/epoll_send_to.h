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

class epoll_send_to final {
 public:
  NOT_COPYABLE(epoll_send_to)

  using send_toed_fn = std::function<void(error_code, int)>;
  using init_send_to_fn = std::function<error_code(reactor_event *)>;
  using fin_send_to_fn = std::function<error_code(io_handle fd, uint8_t *buffer, int size,
                                                  const ip_endpoint &peer_ep, int &bytes_sent)>;

  epoll_send_to(epoll_send_to &&tmp) noexcept;
  epoll_send_to &operator=(epoll_send_to &&tmp) noexcept;

  epoll_send_to() = default;
  epoll_send_to(const reactor_io_descriptor_ptr &io_desc,
                const enqueue_for_completion_fn &enqueue_for_completion,
                const enqueue_for_deletion_fn &enqueue_for_deletion,
                const init_send_to_fn &init_send_to, const fin_send_to_fn &fin_send_to) noexcept;
  ~epoll_send_to() noexcept;
  reactor_event *evt() noexcept;
  void send_to(uint8_t *buffer, int size, const ip_endpoint &peer_ep,
               const send_toed_fn &cb) noexcept;

 private:
  std::shared_ptr<std::mutex> _mtx;
  reactor_event *_evt = nullptr;
  enqueue_for_deletion_fn _enqueue_for_deletion;
  init_send_to_fn _init_send_to;
  fin_send_to_fn _fin_send_to;
};

}  // namespace baba::os