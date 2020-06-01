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

class epoll_send final {
 public:
  NOT_COPYABLE(epoll_send)

  using sended_fn = std::function<void(error_code, int)>;
  using init_send_fn = std::function<error_code(reactor_event *)>;
  using fin_send_fn =
      std::function<error_code(io_handle fd, const uint8_t *buffer, int size, int &bytes_sent)>;

  epoll_send(epoll_send &&tmp) noexcept;
  epoll_send &operator=(epoll_send &&tmp) noexcept;

  epoll_send() = default;
  epoll_send(const reactor_io_descriptor_ptr &io_desc, completion_strand_impl *strand,
             event_registrar *registrar, const enqueue_for_deletion_fn &enqueue_for_deletion,
             const init_send_fn &init_send, const fin_send_fn &fin_send) noexcept;
  void dispose() noexcept;

  void send(const uint8_t *buffer, int size, const sended_fn &cb) noexcept;

  reactor_event *evt() noexcept;

 private:
  event_registrar *_registrar = nullptr;
  reactor_event *_evt = nullptr;
  enqueue_for_deletion_fn _enqueue_for_deletion;
  init_send_fn _init_send;
  fin_send_fn _fin_send;
};

}  // namespace baba::os