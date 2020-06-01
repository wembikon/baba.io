/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/ip_endpoint.h"

#include "ip_address_pimpl.h"

#include "os/socket/ip_endpoint.h"

namespace baba {

struct ip_endpoint::pimpl final {
  pimpl(const ip_endpoint &other) noexcept : impl(other._pimpl->impl) {}
  pimpl(ip_endpoint &&tmp) noexcept : impl(std::move(tmp._pimpl->impl)) {}
  pimpl() noexcept = default;
  pimpl(const std::string &address, uint16_t port) noexcept : impl(address, port) {}
  pimpl(const ip_address &ip, uint16_t port) noexcept : impl(ip._pimpl->impl, port) {}
  pimpl(ip_address &&ip, uint16_t port) noexcept : impl(std::move(ip._pimpl->impl), port) {}
  os::ip_endpoint impl;
};

}  // namespace baba