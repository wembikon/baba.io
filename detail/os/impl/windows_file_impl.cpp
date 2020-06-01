/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/windows_file_impl.h"

#include "baba/io_types.h"

namespace windows_file_impl {

void close(io_handle fd) noexcept {
  if (fd != INVALID_HANDLE_VALUE) {
    CloseHandle(fd);
  }
}

io_result read(io_handle fd, uint8_t *buffer, int size) noexcept {
  int bytes = 0;
  if (!ReadFile(fd, buffer, size, (LPDWORD)&bytes, NULL)) {
    return {(error_code)GetLastError(), -1};
  }
  return {ec::OK, bytes};
}

io_result write(io_handle fd, const uint8_t *buffer, int size) noexcept {
  int bytes = 0;
  if (!WriteFile(fd, buffer, size, (LPDWORD)&bytes, NULL)) {
    return {(error_code)GetLastError(), -1};
  }
  return {ec::OK, bytes};
}

}  // namespace windows_file_impl