/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/kqueue/kqueue_interrupter.h"

#include "baba/io_types.h"
#include "baba/logger.h"

#include <cstdlib>

namespace baba::os {

kqueue_interrupter::~kqueue_interrupter() noexcept {
  // No need to delete file descriptors in kqueue. It will automatically deleted
  // when the last descriptor is closed

  // The read end will be taken cared off by the descriptor
  ::close(_self_pipe_fds[1]);
}

void kqueue_interrupter::init(io_handle kqueue_fd) noexcept {
  _kqueue_fd = kqueue_fd;

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

  _read_event.reset(new reactor_event());
  _read_event->descriptor.reset(new reactor_io_descriptor());
  _read_event->descriptor->fd = _self_pipe_fds[0];

  struct kevent change_list;
  EV_SET(&change_list, _read_event->descriptor->fd, EVFILT_READ, EV_ADD, 0, 0, _read_event.get());
  if (kevent(_kqueue_fd, &change_list, 1, NULL, 0, NULL) == -1) {
    LOGFTL("Failed adding self pipe to kqueue {} for fd {}", _kqueue_fd,
           _read_event->descriptor->fd);
    std::abort();
  }
}

void kqueue_interrupter::interrupt() noexcept {
  if (::write(_self_pipe_fds[1], "x", 1) == -1 && errno != EAGAIN) {
    LOGFTL("Failed writing self pipe write end {} for fd {}", _kqueue_fd, _self_pipe_fds[1]);
    std::abort();
  }
}

io_handle kqueue_interrupter::read_fd() noexcept { return _self_pipe_fds[0]; }

}  // namespace baba::os