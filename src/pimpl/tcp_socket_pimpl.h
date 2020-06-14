/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
 **/

#pragma once

#include "baba/tcp_socket.h"

#include "io_strand_pimpl.h"
#include "ip_endpoint_pimpl.h"
#include "tcp_connector_pimpl.h"

#include "baba/platform.h"

#if defined(PLATFORM_WINDOWS)

#elif defined(PLATFORM_APPLE)

#else
#include "os/reactor/epoll/io/epoll_async_tcp.h"
#endif

namespace baba {

struct tcp_socket::pimpl final {
  pimpl() noexcept = default;
  pimpl(io_strand &strand, io_handle fd, const ip_endpoint &ep) noexcept
      : impl(strand._pimpl->impl, fd, ep._pimpl->impl) {}
  pimpl(tcp_connector &&connector) noexcept : impl(std::move(connector._pimpl->impl)) {}
  os::async_tcp impl;
};

}  // namespace baba