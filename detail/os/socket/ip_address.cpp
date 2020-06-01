/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/socket/ip_address.h"

#include <algorithm>  // count
#include <cstring>    // memset
#include <sstream>    // stringstream

namespace baba::os {

ip_address::ip_address() noexcept : _af(0) {
  std::memset(&_addr.storage, 0, sizeof(native_address_storage::addr_storage));
}

ip_address::ip_address(const ip_address &other) noexcept : _af(other._af) {
  std::memcpy(&_addr.storage, &other._addr.storage, sizeof(native_address_storage::addr_storage));
}

ip_address &ip_address::operator=(const ip_address &other) noexcept {
  _af = other._af;
  std::memcpy(&_addr.storage, &other._addr.storage, sizeof(native_address_storage::addr_storage));
  return *this;
}

ip_address::ip_address(ip_address &&temp) noexcept : _af(temp._af) {
  std::memcpy(&_addr.storage, &temp._addr.storage, sizeof(native_address_storage::addr_storage));
  std::memset(&temp._addr.storage, 0, sizeof(native_address_storage::addr_storage));
  temp._af = 0;
}

ip_address &ip_address::operator=(ip_address &&temp) noexcept {
  _af = temp._af;
  std::memcpy(&_addr.storage, &temp._addr.storage, sizeof(native_address_storage::addr_storage));
  std::memset(&temp._addr.storage, 0, sizeof(native_address_storage::addr_storage));
  temp._af = 0;
  return *this;
}

ip_address::ip_address(const std::string &address) noexcept : _af(0) {
  if (std::count(std::begin(address), std::end(address), ':') >= 2) {
    inet_pton(AF_INET6, address.c_str(), &_addr.storage.ipv6_addr);
    _af = AF_INET6;
  } else if (std::count(std::begin(address), std::end(address), '.') == 3) {
    inet_pton(AF_INET, address.c_str(), &_addr.storage.ipv4_addr);
    _af = AF_INET;
  }
}

ip_address::ip_address(const in_addr &ipv4_addr) noexcept {
  _addr.storage.ipv4_addr = ipv4_addr;
  _af = AF_INET;
}

ip_address::ip_address(const in6_addr &ipv6_addr) noexcept {
  _addr.storage.ipv6_addr = ipv6_addr;
  _af = AF_INET6;
}

const native_address_storage &ip_address::native_address() const noexcept { return _addr; }

socklen_t ip_address::native_address_length() const noexcept {
  if (_af == AF_INET) {
    return INET_ADDRSTRLEN;
  } else if (_af == AF_INET6) {
    return INET6_ADDRSTRLEN;
  }
  return 0;
}

address_family ip_address::family() const noexcept { return _af; }

std::string ip_address::to_string() const noexcept {
  if (_af == AF_INET) {
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, (void *)&(_addr.storage.ipv4_addr), buf, INET_ADDRSTRLEN);
    return std::string(buf);
  } else if (_af == AF_INET6) {
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, (void *)&(_addr.storage.ipv6_addr), buf, INET6_ADDRSTRLEN);
    return std::string(buf);
  }
  return std::string();
}

bool operator==(const ip_address &lhs, const ip_address &rhs) noexcept {
  return lhs.to_string() == rhs.to_string();
}

bool operator!=(const ip_address &lhs, const ip_address &rhs) noexcept { return !(lhs == rhs); }

}  // namespace baba::os