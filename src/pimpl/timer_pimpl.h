/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
 **/

#pragma once

#include "baba/timer.h"

#include "io_strand_pimpl.h"

#include "baba/platform.h"

#if defined(PLATFORM_WINDOWS)

#elif defined(PLATFORM_APPLE)

#else
#include "os/reactor/epoll/io/epoll_async_timer.h"
#endif

namespace baba {

struct timer::pimpl final {
  pimpl() noexcept = default;
  pimpl(io_strand &strand) noexcept : impl(strand._pimpl->impl) {}
  os::async_timer impl;
};

}  // namespace baba