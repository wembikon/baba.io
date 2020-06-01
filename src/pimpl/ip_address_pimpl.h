/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/ip_address.h"

#include "os/socket/ip_address.h"

namespace baba {

struct ip_address::pimpl final {
  pimpl(const ip_address &other) noexcept : impl(other._pimpl->impl) {}
  pimpl(ip_address &&tmp) noexcept : impl(std::move(tmp._pimpl->impl)) {}
  pimpl() noexcept = default;
  pimpl(const std::string &ip) noexcept : impl(ip) {}
  os::ip_address impl;
};

}  // namespace baba