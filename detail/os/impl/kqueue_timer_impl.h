/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

/**
 * Kqueue only needs an ident (not a file descriptor, but just an incrementing integer
 * to associate the timer) for EVFILT_TIMER.
 *
 * From BSD manual:
 * ident - Value used to identify this event.  The	exact interpretation
 *         is determined by the attached filter, but often	is a file descriptor.
 **/

#include "baba/io_handle.h"

#include <cstdint>

namespace baba::os {

class kqueue_timer_impl {
 public:
  static io_handle create() noexcept;

 private:
  // We use a smaller than int integer type here so that it will wrap around safely
  static uint16_t _ident_generator;
};

}  // namespace baba::os