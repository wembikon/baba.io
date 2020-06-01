/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"

#include <cstdint>

namespace windows_file_impl {

void close(io_handle fd) noexcept;

struct io_result {
  error_code ec = ec::OK;
  int bytes = 0;
};

io_result read(io_handle fd, uint8_t *buffer, int size) noexcept;
io_result write(io_handle fd, uint8_t *buffer, int size) noexcept;

}  // namespace windows_file_impl