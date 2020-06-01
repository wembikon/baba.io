/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "os/common/event_registrar.h"
#include "os/reactor/epoll/epoll_completion_strand.h"
#include "os/reactor/epoll/epoll_reactor_event.h"

namespace baba::os {

class epoll_recv final {
 public:
  NOT_COPYABLE(epoll_recv)

  using recved_fn = std::function<void(error_code, int)>;
  using init_recv_fn = std::function<error_code(reactor_event *)>;
  using fin_recv_fn =
      std::function<error_code(io_handle fd, uint8_t *buffer, int size, int &bytes_recv)>;

  epoll_recv(epoll_recv &&tmp) noexcept;
  epoll_recv &operator=(epoll_recv &&tmp) noexcept;

  epoll_recv() = default;
  epoll_recv(const reactor_io_descriptor_ptr &io_desc, completion_strand_impl *strand,
             event_registrar *registrar, const enqueue_for_deletion_fn &enqueue_for_deletion,
             const init_recv_fn &init_recv, const fin_recv_fn &fin_recv) noexcept;
  void dispose() noexcept;

  void recv(uint8_t *buffer, int size, const recved_fn &cb) noexcept;

  reactor_event *evt() noexcept;

 private:
  event_registrar *_registrar = nullptr;
  reactor_event *_evt = nullptr;
  enqueue_for_deletion_fn _enqueue_for_deletion;
  init_recv_fn _init_recv;
  fin_recv_fn _fin_recv;
};

}  // namespace baba::os