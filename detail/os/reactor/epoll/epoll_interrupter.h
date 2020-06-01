/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "os/impl/unix_file_impl.h"
#include "os/reactor/epoll/epoll_reactor_event.h"

namespace baba::os {

class epoll_interrupter final {
 public:
  NOT_COPYABLE(epoll_interrupter)
  NOT_MOVEABLE(epoll_interrupter)

  epoll_interrupter() = default;
  void close() noexcept;
  void init(io_handle epoll_fd) noexcept;
  void interrupt() noexcept;
  io_handle read_fd() noexcept;

 private:
  io_handle _epoll_fd;
  io_handle _self_pipe_fds[2];
  reactor_io_descriptor_ptr _read_descriptor;
};

}  // namespace baba::os