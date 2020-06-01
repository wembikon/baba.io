/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/address_family.h"
#include "baba/io_types.h"

#include <cstdint>
#include <string>

namespace baba::os {

class native_address_storage {
 private:
  union addr_storage {
    in_addr ipv4_addr;
    in6_addr ipv6_addr;
  };
  addr_storage storage;
  friend class ip_address;

 public:
  inline const in_addr &ipv4() const noexcept { return storage.ipv4_addr; }
  inline const in6_addr &ipv6() const noexcept { return storage.ipv6_addr; }
};

class ip_address {
 public:
  static ip_address any() noexcept { return ip_address("0.0.0.0"); }

  // Yields uninitialized value
  ip_address() noexcept;
  // Copy
  ip_address(const ip_address &other) noexcept;
  ip_address &operator=(const ip_address &other) noexcept;
  // Move
  ip_address(ip_address &&temp) noexcept;
  ip_address &operator=(ip_address &&temp) noexcept;

  explicit ip_address(const std::string &address) noexcept;
  ip_address(const in_addr &ipv4_addr) noexcept;
  ip_address(const in6_addr &ipv6_addr) noexcept;

  const native_address_storage &native_address() const noexcept;
  socklen_t native_address_length() const noexcept;

  address_family family() const noexcept;
  std::string to_string() const noexcept;

 private:
  void init(uint16_t af, uint16_t port) noexcept;

 private:
  native_address_storage _addr;
  uint16_t _af;
};  // class

bool operator==(const ip_address &lhs, const ip_address &rhs) noexcept;
bool operator!=(const ip_address &lhs, const ip_address &rhs) noexcept;

}  // namespace baba::os