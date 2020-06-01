/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/address_family.h"

#include <memory>
#include <string>

namespace baba {

class ip_address final {
 public:
  struct pimpl;

  ip_address(const ip_address &other) noexcept;
  ip_address &operator=(const ip_address &other) noexcept;
  ip_address(ip_address &&tmp) noexcept;
  ip_address &operator=(ip_address &&tmp) noexcept;
  ip_address() noexcept;
  ~ip_address() noexcept;

  ip_address(const std::string &ip) noexcept;
  address_family family() const noexcept;
  std::string to_string() const noexcept;

 private:
  friend class ip_endpoint;
  friend class set_outgoing_interface;
  friend class join_group;
  std::unique_ptr<pimpl> _pimpl;
};

bool operator==(const ip_address &lhs, const ip_address &rhs) noexcept;
bool operator!=(const ip_address &lhs, const ip_address &rhs) noexcept;

}  // namespace baba