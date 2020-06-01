/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "ip_endpoint_pimpl.h"

#include "os/socket/ip_endpoint.h"

namespace baba {

ip_endpoint::ip_endpoint(const ip_endpoint &other) noexcept : _pimpl(new pimpl(other)) {}

ip_endpoint &ip_endpoint::operator=(const ip_endpoint &other) noexcept {
  _pimpl->impl = other._pimpl->impl;
  return *this;
}

ip_endpoint::ip_endpoint(ip_endpoint &&tmp) noexcept : _pimpl(new pimpl(std::move(tmp))) {}

ip_endpoint &ip_endpoint::operator=(ip_endpoint &&tmp) noexcept {
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

ip_endpoint::ip_endpoint() noexcept : _pimpl(new pimpl()) {}

ip_endpoint::~ip_endpoint() noexcept = default;

ip_endpoint::ip_endpoint(const std::string &ip_str, uint16_t port) noexcept
    : _pimpl(new pimpl(ip_str, port)) {}

ip_endpoint::ip_endpoint(const ip_address &ip, uint16_t port) noexcept
    : _pimpl(new pimpl(ip, port)) {}

ip_endpoint::ip_endpoint(ip_address &&ip, uint16_t port) noexcept
    : _pimpl(new pimpl(std::move(ip), port)) {}

ip_address ip_endpoint::address() const noexcept {
  ip_address ip;
  ip._pimpl->impl = _pimpl->impl.address();
  return ip;
}

uint16_t ip_endpoint::port() const noexcept { return _pimpl->impl.port(); }

std::string ip_endpoint::to_string() const noexcept { return _pimpl->impl.to_string(); }

bool operator==(const ip_endpoint &lhs, const ip_endpoint &rhs) noexcept {
  return lhs.to_string() == rhs.to_string();
}

bool operator!=(const ip_endpoint &lhs, const ip_endpoint &rhs) noexcept { return !(lhs == rhs); }

}  // namespace baba