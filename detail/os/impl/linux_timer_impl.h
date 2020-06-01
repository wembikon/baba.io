/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"

#include <cstdint>

namespace baba::os::linux_timer_impl {

struct create_timer_result final {
  error_code ec = ec::OK;
  io_handle fd = INVALID_HANDLE_VALUE;
};

create_timer_result open() noexcept;

/**
 * Arm the timer. It will fire EPOLLIN once it has expired
 */
error_code init_timer(io_handle fd, uint32_t timeout_ms) noexcept;

}  // namespace baba::os::linux_timer_impl