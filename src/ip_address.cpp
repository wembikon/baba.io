/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "ip_address_pimpl.h"

namespace baba {

ip_address::ip_address(const ip_address &other) noexcept : _pimpl(new pimpl(other)) {}

ip_address &ip_address::operator=(const ip_address &other) noexcept {
  _pimpl->impl = other._pimpl->impl;
  return *this;
}

ip_address::ip_address(ip_address &&tmp) noexcept : _pimpl(new pimpl(std::move(tmp))) {}

ip_address &ip_address::operator=(ip_address &&tmp) noexcept {
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

ip_address::ip_address() noexcept : _pimpl(new pimpl()) {}

ip_address::~ip_address() noexcept = default;

ip_address::ip_address(const std::string &ip) noexcept : _pimpl(new pimpl(ip)) {}

address_family ip_address::family() const noexcept { return _pimpl->impl.family(); }

std::string ip_address::to_string() const noexcept { return _pimpl->impl.to_string(); }

bool operator==(const ip_address &lhs, const ip_address &rhs) noexcept {
  return lhs.to_string() == rhs.to_string();
}

bool operator!=(const ip_address &lhs, const ip_address &rhs) noexcept { return !(lhs == rhs); }

}  // namespace baba