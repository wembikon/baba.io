/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/io_service.h"

#include "baba/platform.h"

#if defined(PLATFORM_WINDOWS)

#elif defined(PLATFORM_APPLE)

#else
#include "os/reactor/epoll/epoll_service.h"
using platform_io_service = baba::os::epoll_service;
#endif

namespace baba {

struct io_service::pimpl final {
  pimpl(int reactor_threads) noexcept : impl(reactor_threads) {}
  platform_io_service impl;
};

}  // namespace baba