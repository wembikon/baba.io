/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/epoll_interrupter.h"

#include "baba/io_types.h"
#include "baba/logger.h"

#include <cstdlib>

namespace baba::os {

void epoll_interrupter::close() noexcept {
  struct epoll_event evt;
  if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _self_pipe_fds[0], &evt) == -1) {
    LOGFTL("Failed deleting self pipe read end from epoll {} for fd {}. ec={}", _epoll_fd,
           _self_pipe_fds[0], errno);
    std::abort();
  }

  ::close(_self_pipe_fds[0]);
  ::close(_self_pipe_fds[1]);
}

void epoll_interrupter::init(io_handle epoll_fd) noexcept {
  _epoll_fd = epoll_fd;

  if (::pipe(_self_pipe_fds) == -1) {
    LOGFTL("Failed to instantiate self pipe. error={}", errno);
    std::abort();
  }
  // Make read and write end of self pipe non-blocking
  int flags = fcntl(_self_pipe_fds[0], F_GETFL);
  if (flags == -1) {
    LOGFTL("Self pipe get read flag failed. error={}", errno);
    std::abort();
  }
  flags |= O_NONBLOCK;
  if (fcntl(_self_pipe_fds[0], F_SETFL, flags) == -1) {
    LOGFTL("Self pipe set read flag failed. error={}", errno);
    std::abort();
  }
  flags = fcntl(_self_pipe_fds[1], F_GETFL);
  if (flags == -1) {
    LOGFTL("Self pipe set read flag failed. error={}", errno);
    std::abort();
  }
  flags |= O_NONBLOCK;
  if (fcntl(_self_pipe_fds[1], F_SETFL, flags) == -1) {
    LOGFTL("Self pipe set write flag failed. error={}", errno);
    std::abort();
  }

  _read_descriptor.reset(new reactor_io_descriptor());
  _read_descriptor->fd = _self_pipe_fds[0];

  struct epoll_event evt;
  evt.events = EPOLLIN;
  evt.data.ptr = _read_descriptor.get();
  if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _read_descriptor->fd, &evt) == -1) {
    LOGFTL("Failed adding self pipe to epoll {} for fd {}. ec={}", _epoll_fd, _read_descriptor->fd,
           errno);
    std::abort();
  }
}

void epoll_interrupter::interrupt() noexcept {
  if (::write(_self_pipe_fds[1], "x", 1) == -1 && errno != EAGAIN) {
    LOGFTL("Failed writing self pipe write end {} for fd {}. ec={}", _epoll_fd, _self_pipe_fds[1],
           errno);
    std::abort();
  }
}

io_handle epoll_interrupter::read_fd() noexcept { return _self_pipe_fds[0]; }

}  // namespace baba::os