/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/io_types.h"
#include "os/reactor/iocp/iocp_reactor_event.h"

#include <cstdint>

namespace baba::os::windows_timer_impl {

struct iocp_timer_context final {
  io_handle iocp_fd = INVALID_HANDLE_VALUE;
  reactor_event *evt = nullptr;
};

struct create_timer_result final {
  error_code ec = ec::OK;
  io_handle fd = INVALID_HANDLE_VALUE;
};

create_timer_result open(iocp_timer_context *ctx) noexcept;

void close(io_handle fd) noexcept;

error_code init_timer(io_handle fd, uint32_t timeout_ms) noexcept;

}  // namespace baba::os::windows_timer_impl