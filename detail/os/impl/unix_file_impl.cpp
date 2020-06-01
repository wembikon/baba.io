/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/unix_file_impl.h"

#include "baba/io_types.h"

namespace baba::unix_file_impl {

void close(io_handle fd) noexcept { ::close(fd); }

io_result read(io_handle fd, uint8_t *buffer, int size) noexcept {
  const int bytes = ::read(fd, buffer, size);
  if (bytes == -1) {
    return {errno, -1};
  }
  return {ec::OK, bytes};
}

io_result write(io_handle fd, const uint8_t *buffer, int size) noexcept {
  const int bytes = ::write(fd, buffer, size);
  if (bytes == -1) {
    return {errno, -1};
  }
  return {ec::OK, bytes};
}

}  // namespace baba::unix_file_impl