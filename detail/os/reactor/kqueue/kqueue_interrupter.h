/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"
#include "os/impl/unix_file_impl.h"
#include "os/reactor/kqueue/kqueue_reactor_event.h"

#include <memory>

namespace baba::os {

class kqueue_interrupter final {
 public:
  NOT_COPYABLE(kqueue_interrupter)
  NOT_MOVEABLE(kqueue_interrupter)

  kqueue_interrupter() = default;
  ~kqueue_interrupter() noexcept;
  void init(io_handle kqueue_fd) noexcept;
  void interrupt() noexcept;
  io_handle read_fd() noexcept;

 private:
  io_handle _kqueue_fd;
  io_handle _self_pipe_fds[2];
  std::shared_ptr<reactor_event> _read_event;
};

}  // namespace baba::os