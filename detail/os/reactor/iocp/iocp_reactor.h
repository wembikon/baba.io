/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/io_handle.h"
#include "baba/semantics.h"
#include "os/reactor/iocp/iocp_reactor_event.h"

#include <deque>

#define MAX_IOCP_ENTRIES 100

namespace baba::os {

class iocp_reactor final {
 public:
  NOT_COPYABLE(iocp_reactor)
  NOT_MOVEABLE(iocp_reactor)

  iocp_reactor() noexcept;

  /**
   * Wait for events to be available
   *
   * Returns:
   * - a reactor error. Might not be recoverable
   **/
  error_code wait() noexcept;

  /**
   * Closing the handle will also interrupt the blocking wait
   **/
  void close() noexcept;

  /**
   * Needed by interrupter
   **/
  io_handle fd() const noexcept;

 private:
  io_handle _fd = INVALID_HANDLE_VALUE;
  OVERLAPPED_ENTRY _event_list[MAX_IOCP_ENTRIES];
};

}  // namespace baba::os