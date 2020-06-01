/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/iocp_impl.h"

#include "baba/io_types.h"
#include "baba/logger.h"

namespace baba::os::iocp_impl {

create_iocp_result create() noexcept {
  const auto fd = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
  if (fd == NULL) {
    return {(error_code)GetLastError(), INVALID_HANDLE_VALUE};
  }
  return {ec::OK, fd};
}

void reg(io_handle iocp_fd, io_handle fd) noexcept { CreateIoCompletionPort(fd, iocp_fd, NULL, 0); }

wait_result wait(io_handle iocp_fd, OVERLAPPED_ENTRY *event_list, int size) noexcept {
  wait_result result;
  if (GetQueuedCompletionStatusEx(iocp_fd, event_list, size, &result.ready_io_count, INFINITE,
                                  false) == 0) {
    result.ec = GetLastError();
  }
  return result;
}

}  // namespace baba::os::iocp_impl