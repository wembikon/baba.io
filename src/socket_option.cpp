/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "socket_option_pimpl.h"

#include "os/socket/socket_option.h"

namespace baba {

// Reuse address

reuse_address::reuse_address(const reuse_address &other) noexcept : _pimpl(new pimpl(other)) {}

reuse_address &reuse_address::operator=(const reuse_address &other) noexcept {
  _pimpl->impl = other._pimpl->impl;
  return *this;
}

reuse_address::reuse_address(reuse_address &&tmp) noexcept : _pimpl(new pimpl(std::move(tmp))) {}

reuse_address &reuse_address::operator=(reuse_address &&tmp) noexcept {
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

reuse_address::reuse_address() noexcept : _pimpl(new pimpl()) {}

reuse_address::~reuse_address() noexcept = default;

reuse_address::reuse_address(bool yes) noexcept : _pimpl(new pimpl(yes)) {}

// Set outgoing interface

set_outgoing_interface::set_outgoing_interface(const set_outgoing_interface &other) noexcept
    : _pimpl(new pimpl(other)) {}

set_outgoing_interface &set_outgoing_interface::operator=(
    const set_outgoing_interface &other) noexcept {
  _pimpl->impl = other._pimpl->impl;
  return *this;
}

set_outgoing_interface::set_outgoing_interface(set_outgoing_interface &&tmp) noexcept
    : _pimpl(new pimpl(std::move(tmp))) {}

set_outgoing_interface &set_outgoing_interface::operator=(set_outgoing_interface &&tmp) noexcept {
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

set_outgoing_interface::set_outgoing_interface() noexcept : _pimpl(new pimpl()) {}

set_outgoing_interface::~set_outgoing_interface() noexcept = default;

set_outgoing_interface::set_outgoing_interface(const ip_address &addr) noexcept
    : _pimpl(new pimpl(addr)) {}

// Join group

join_group::join_group(const join_group &other) noexcept : _pimpl(new pimpl(other)) {}

join_group &join_group::operator=(const join_group &other) noexcept {
  _pimpl->impl = other._pimpl->impl;
  return *this;
}

join_group::join_group(join_group &&tmp) noexcept : _pimpl(new pimpl(std::move(tmp))) {}

join_group &join_group::operator=(join_group &&tmp) noexcept {
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

join_group::join_group() noexcept : _pimpl(new pimpl()) {}

join_group::~join_group() noexcept = default;

join_group::join_group(const ip_address &iface, uint32_t iface_index,
                       const ip_address &mcast) noexcept
    : _pimpl(new pimpl(iface, iface_index, mcast)) {}

namespace socket {

error_code set_option(io_handle fd, const reuse_address &opt) noexcept {
  return os::socket::set_option(fd, opt._pimpl->impl);
}

error_code set_option(io_handle fd, const set_outgoing_interface &opt) noexcept {
  return os::socket::set_option(fd, opt._pimpl->impl);
}

error_code set_option(io_handle fd, const join_group &opt) noexcept {
  return os::socket::set_option(fd, opt._pimpl->impl);
}

error_code get_option(io_handle fd, reuse_address &opt) noexcept {
  return os::socket::get_option(fd, opt._pimpl->impl);
}

error_code get_option(io_handle fd, set_outgoing_interface &opt) noexcept {
  return os::socket::get_option(fd, opt._pimpl->impl);
}

error_code get_option(io_handle fd, join_group &opt) noexcept {
  return os::socket::get_option(fd, opt._pimpl->impl);
}

}  // namespace socket

}  // namespace baba