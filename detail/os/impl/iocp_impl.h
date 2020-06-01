/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/io_types.h"

#include <cstdint>

namespace baba::os::iocp_impl {

struct create_iocp_result final {
  error_code ec = ec::OK;
  io_handle fd = INVALID_HANDLE_VALUE;
};

/**
 * Create an iocp object
 **/
create_iocp_result create() noexcept;

/**
 * Associate a file handle to an iocp object
 **/
void reg(io_handle iocp_fd, io_handle fd) noexcept;

/**
 * No need to delete file descriptors in iocp. It will automatically deleted
 * when the last descriptor is closed
 **/

/**
 * Block until an event is available
 **/
struct wait_result final {
  error_code ec = ec::OK;
  ULONG ready_io_count = 0;
};

wait_result wait(io_handle iocp_fd, OVERLAPPED_ENTRY *event_list, int size) noexcept;

}  // namespace baba::os::iocp_impl