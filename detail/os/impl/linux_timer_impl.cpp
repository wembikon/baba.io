/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/linux_timer_impl.h"
#include "baba/io_types.h"

namespace baba::os::linux_timer_impl {

create_timer_result open() noexcept {
  if (const auto fd = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC); fd == INVALID_HANDLE_VALUE) {
    return {errno, INVALID_HANDLE_VALUE};
  } else {
    return {ec::OK, fd};
  }
}

error_code init_timer(io_handle fd, uint32_t timeout_ms) noexcept {
  constexpr uint32_t timescale_us = 1000000;
  constexpr uint32_t timescale_ms = 1000;
  struct itimerspec expiration;
  expiration.it_interval.tv_sec = 0;
  expiration.it_interval.tv_nsec = 0;
  expiration.it_value.tv_sec = timeout_ms / timescale_ms;
  expiration.it_value.tv_nsec = (timeout_ms % timescale_ms) * timescale_us;
  if (::timerfd_settime(fd, 0, &expiration, NULL) == -1) {
    return errno;
  }
  return ec::OK;
}

}  // namespace baba::os::linux_timer_impl