/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/epoll_impl.h"

#include "baba/io_types.h"
#include "baba/logger.h"

namespace baba::os::epoll_impl {

create_epoll_result create() noexcept {
  if (const auto fd = epoll_create1(O_CLOEXEC); fd == INVALID_HANDLE_VALUE) {
    return {errno, INVALID_HANDLE_VALUE};
  } else {
    return {ec::OK, fd};
  }
}

error_code arm(io_handle epoll_fd, io_handle fd, uint32_t events, void *data) noexcept {
  struct epoll_event evt;
  evt.events = events | EPOLLONESHOT;
  evt.data.ptr = data;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &evt) == -1) {
    if (errno != ENOENT) {
      return errno;
    }
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &evt) == -1) {
      LOGFTL("Failed EPOLL_CTL_ADD on epoll {} for fd {}, ec={}", epoll_fd, fd, errno);
      return errno;
    }
  }
  return ec::OK;
}

void remove(io_handle epoll_fd, io_handle fd) noexcept {
  struct epoll_event evt;
  if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &evt) == -1 && errno != ENOENT) {
    LOGFTL("Failed EPOLL_CTL_DEL on epoll {} for fd {}, ec={}", epoll_fd, fd, errno);
  }
}

wait_result wait(io_handle epoll_fd, struct epoll_event *event_list, int size) noexcept {
  wait_result result;
  result.ready_io_count = epoll_wait(epoll_fd, event_list, size, -1);
  // If got interrupted by a signal we will not treat it as an error
  if (result.ready_io_count == -1 && errno != EINTR) {
    result.ec = errno;
  }
  return result;
}

}  // namespace baba::os::epoll_impl