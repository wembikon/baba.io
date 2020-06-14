/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
 **/

#pragma once

#include "baba/tcp_connector.h"

#include "io_strand_pimpl.h"

#include "baba/platform.h"

#if defined(PLATFORM_WINDOWS)

#elif defined(PLATFORM_APPLE)

#else
#include "os/reactor/epoll/io/epoll_async_connector.h"
#endif

namespace baba {

struct tcp_connector::pimpl final {
  pimpl() noexcept = default;
  pimpl(io_strand &strand, address_family af) noexcept : impl(strand._pimpl->impl, af) {}
  os::async_connector impl;
};

}  // namespace baba