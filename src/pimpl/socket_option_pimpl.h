/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/socket_option.h"

#include "ip_address_pimpl.h"

#include "os/socket/socket_option.h"

namespace baba {

struct reuse_address::pimpl final {
  pimpl(const reuse_address &other) noexcept : impl(other._pimpl->impl) {}
  pimpl(reuse_address &&tmp) noexcept : impl(std::move(tmp._pimpl->impl)) {}
  pimpl() = default;
  pimpl(bool yes) noexcept : impl(yes) {}
  os::reuse_address impl;
};

struct set_outgoing_interface::pimpl final {
  pimpl(const set_outgoing_interface &other) noexcept : impl(other._pimpl->impl) {}
  pimpl(set_outgoing_interface &&tmp) noexcept : impl(std::move(tmp._pimpl->impl)) {}
  pimpl() = default;
  pimpl(const ip_address &addr) noexcept : impl(addr._pimpl->impl) {}
  os::set_outgoing_interface impl;
};

struct join_group::pimpl final {
  pimpl(const join_group &other) noexcept : impl(other._pimpl->impl) {}
  pimpl(join_group &&tmp) noexcept : impl(std::move(tmp._pimpl->impl)) {}
  pimpl() = default;
  pimpl(const ip_address &iface, uint32_t iface_index, const ip_address &mcast) noexcept
      : impl(iface._pimpl->impl, iface_index, mcast._pimpl->impl) {}
  os::join_group impl;
};

}  // namespace baba