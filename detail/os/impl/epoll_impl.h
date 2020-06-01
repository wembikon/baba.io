/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/io_types.h"

#include <cstdint>

namespace baba::os::epoll_impl {

struct create_epoll_result final {
  error_code ec = ec::OK;
  io_handle fd = INVALID_HANDLE_VALUE;
};

/**
 * Create an epoll object
 **/
create_epoll_result create() noexcept;

/**
 * Registration is done when arming the event
 **/

/**
 * Listen for events
 **/
error_code arm(io_handle epoll_fd, io_handle fd, uint32_t events, void *data) noexcept;

/**
 * Remove fd from epoll. Consume error.
 **/
void remove(io_handle epoll_fd, io_handle fd) noexcept;

/**
 * Block until an event is available
 **/
struct wait_result final {
  error_code ec = ec::OK;
  int ready_io_count = 0;
};

wait_result wait(io_handle epoll_fd, struct epoll_event *event_list, int size) noexcept;

}  // namespace baba::os::epoll_impl