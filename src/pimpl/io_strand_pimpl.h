/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/io_strand.h"

#include "io_service_pimpl.h"

#include "baba/platform.h"

#if defined(PLATFORM_WINDOWS)

#elif defined(PLATFORM_APPLE)

#else
#include "os/reactor/epoll/epoll_completion_strand.h"
using platform_io_strand = baba::os::completion_strand;
#endif

namespace baba {

struct io_strand::pimpl final {
  pimpl(io_service &service) noexcept : impl(service._pimpl->impl) {}
  platform_io_strand impl;
};

}  // namespace baba