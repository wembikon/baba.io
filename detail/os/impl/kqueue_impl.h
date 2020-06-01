/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/io_types.h"

#include <cstdint>

namespace baba::os::kqueue_impl {

struct create_kqueue_result final {
  error_code ec = ec::OK;
  io_handle fd = INVALID_HANDLE_VALUE;
};

/**
 * Create an kqueue object
 **/
create_kqueue_result create() noexcept;

/**
 * Registration is done when arming the event
 **/

/**
 * Listen for events
 **/
error_code arm(io_handle kqueue_fd, io_handle fd, int event_filter, unsigned int fflags,
               int64_t data, void *udata) noexcept;

/**
 * No need to delete file descriptors in kqueue. It will automatically deleted
 * when the last descriptor is closed
 **/

/**
 * Block until an event is available
 **/
struct wait_result final {
  error_code ec = ec::OK;
  int ready_io_count = 0;
};

wait_result wait(io_handle kqueue_fd, struct kevent *event_list, int size) noexcept;

}  // namespace baba::os::kqueue_impl