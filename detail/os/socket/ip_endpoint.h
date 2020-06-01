/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/socket/ip_address.h"

#include <cstdint>
#include <string>

namespace baba::os {

class native_socket_storage {
 private:
  sockaddr_storage storage;
  friend class ip_endpoint;

 public:
  inline const sockaddr_in &ipv4() const noexcept { return (sockaddr_in &)storage; }
  inline const sockaddr_in6 &ipv6() const noexcept { return (sockaddr_in6 &)storage; }
};

class ip_endpoint {
 public:
  static ip_endpoint any() noexcept { return ip_endpoint(ip_address("0.0.0.0"), 0); }

  // Yields uninitialized value
  ip_endpoint() noexcept;
  // Copy
  ip_endpoint(const ip_endpoint &other) noexcept;
  ip_endpoint &operator=(const ip_endpoint &other) noexcept;
  // Move
  ip_endpoint(ip_endpoint &&temp) noexcept;
  ip_endpoint &operator=(ip_endpoint &&temp) noexcept;

  ip_endpoint(const std::string &address, uint16_t port) noexcept;
  ip_endpoint(const ip_address &address, uint16_t port) noexcept;
  ip_endpoint(ip_address &&address, uint16_t port) noexcept;
  ip_endpoint(const sockaddr_in &ipv4_sock) noexcept;
  ip_endpoint(const sockaddr_in6 &ipv6_sock) noexcept;

  const native_socket_storage &native_socket() const noexcept;
  socklen_t native_socket_length() const noexcept;

  const ip_address &address() const noexcept;
  uint16_t port() const noexcept;
  std::string to_string() const noexcept;

 private:
  void init_native_socket() noexcept;

 private:
  native_socket_storage _sock;
  socklen_t _sock_len;
  ip_address _address;
  uint16_t _port;
};  // class

bool operator==(const ip_endpoint &lhs, const ip_endpoint &rhs) noexcept;
bool operator!=(const ip_endpoint &lhs, const ip_endpoint &rhs) noexcept;

}  // namespace baba::os