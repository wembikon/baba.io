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

class epoll_timer final {
 public:
  NOT_COPYABLE(epoll_timer)

  using expired_fn = std::function<void(error_code)>;
  using init_timer_fn = std::function<error_code(reactor_event *, uint32_t)>;
  using fin_timer_fn =
      std::function<error_code(io_handle fd, uint8_t *buffer, int size, int &bytes_read)>;

  epoll_timer(epoll_timer &&tmp) noexcept;
  epoll_timer &operator=(epoll_timer &&tmp) noexcept;

  epoll_timer() = default;
  epoll_timer(const reactor_io_descriptor_ptr &io_desc, completion_strand_impl *strand,
              event_registrar *registrar, const enqueue_for_deletion_fn &enqueue_for_deletion,
              const init_timer_fn &init_timer, const fin_timer_fn &fin_timer) noexcept;
  void dispose() noexcept;

  void timeout(uint32_t timeout_ms, const expired_fn &cb) noexcept;

  reactor_event *evt() noexcept;

 private:
  event_registrar *_registrar = nullptr;
  reactor_event *_evt = nullptr;
  enqueue_for_deletion_fn _enqueue_for_deletion;
  init_timer_fn _init_timer;
  fin_timer_fn _fin_timer;
};

}  // namespace baba::os