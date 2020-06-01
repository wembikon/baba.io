/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/kqueue_impl.h"

#include "baba/io_types.h"
#include "baba/logger.h"

namespace baba::os::kqueue_impl {

create_kqueue_result create() noexcept {
  if (const auto fd = kqueue(); fd == -1) {
    return {errno, -1};
  } else {
    return {ec::OK, fd};
  }
}

error_code arm(io_handle kqueue_fd, io_handle fd, int event_filter, unsigned int fflags,
               int64_t data, void *udata) noexcept {
  struct kevent change_list;
  EV_SET(&change_list, fd, event_filter, EV_ADD | EV_ONESHOT, fflags, data, udata);
  if (kevent(kqueue_fd, &change_list, 1, NULL, 0, NULL) == -1) {
    LOGFTL("Failed EV_ADD on kqueue {} for fd {}. error={}", kqueue_fd, fd, errno);
    return errno;
  }
  return ec::OK;
}

wait_result wait(io_handle kqueue_fd, struct kevent *event_list, int size) noexcept {
  wait_result result;
  result.ready_io_count = kevent(kqueue_fd, NULL, 0, event_list, size, NULL);
  // If got interrupted by a signal we will not treat it as an error
  if (result.ready_io_count == -1 && errno != EINTR) {
    result.ec = errno;
  }
  return result;
}

}  // namespace baba::os::kqueue_impl