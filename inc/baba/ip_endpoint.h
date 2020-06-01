/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/ip_address.h"

#include <memory>
#include <string>

namespace baba {

class ip_endpoint final {
 public:
  struct pimpl;

  ip_endpoint(const ip_endpoint &other) noexcept;
  ip_endpoint &operator=(const ip_endpoint &other) noexcept;
  ip_endpoint(ip_endpoint &&tmp) noexcept;
  ip_endpoint &operator=(ip_endpoint &&tmp) noexcept;
  ip_endpoint() noexcept;
  ~ip_endpoint() noexcept;

  ip_endpoint(const std::string &ip_str, uint16_t port) noexcept;
  ip_endpoint(const ip_address &ip, uint16_t port) noexcept;
  ip_endpoint(ip_address &&ip, uint16_t port) noexcept;
  ip_address address() const noexcept;
  uint16_t port() const noexcept;
  std::string to_string() const noexcept;

 private:
  friend class tcp_connector;
  friend class tcp_socket;
  friend class tcp_acceptor;
  std::unique_ptr<pimpl> _pimpl;
};

bool operator==(const ip_endpoint &lhs, const ip_endpoint &rhs) noexcept;
bool operator!=(const ip_endpoint &lhs, const ip_endpoint &rhs) noexcept;

}  // namespace baba