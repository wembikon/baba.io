/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/socket/ip_endpoint.h"

#include <algorithm>  // count
#include <cstring>    // memset
#include <sstream>    // stringstream

namespace baba::os {

ip_endpoint::ip_endpoint() noexcept : _sock_len(0), _port(0) {
  std::memset(&_sock.storage, 0, sizeof(sockaddr_storage));
}

ip_endpoint::ip_endpoint(const ip_endpoint &other) noexcept
    : _sock_len(other._sock_len), _address(other._address), _port(other._port) {
  std::memcpy(&_sock.storage, &other._sock.storage, sizeof(sockaddr_storage));
}

ip_endpoint &ip_endpoint::operator=(const ip_endpoint &other) noexcept {
  _sock_len = other._sock_len;
  _address = other._address;
  _port = other._port;
  std::memcpy(&_sock.storage, &other._sock.storage, sizeof(sockaddr_storage));
  return *this;
}

ip_endpoint::ip_endpoint(ip_endpoint &&temp) noexcept
    : _sock_len(temp._sock_len), _address(std::move(temp._address)), _port(temp._port) {
  std::memcpy(&_sock.storage, &temp._sock.storage, sizeof(sockaddr_storage));
  std::memset(&temp._sock.storage, 0, sizeof(sockaddr_storage));
  temp._sock_len = 0;
  temp._port = 0;
}

ip_endpoint &ip_endpoint::operator=(ip_endpoint &&temp) noexcept {
  _sock_len = temp._sock_len;
  _address = std::move(temp._address);
  _port = temp._port;
  std::memcpy(&_sock.storage, &temp._sock.storage, sizeof(sockaddr_storage));
  std::memset(&temp._sock.storage, 0, sizeof(sockaddr_storage));
  temp._sock_len = 0;
  temp._port = 0;
  return *this;
}

ip_endpoint::ip_endpoint(const std::string &address, uint16_t port) noexcept
    : ip_endpoint(ip_address(address), port) {}

ip_endpoint::ip_endpoint(const ip_address &address, uint16_t port) noexcept
    : _sock_len(0), _address(address), _port(port) {
  init_native_socket();
}

ip_endpoint::ip_endpoint(ip_address &&address, uint16_t port) noexcept
    : _sock_len(0), _address(std::move(address)), _port(port) {
  init_native_socket();
}

ip_endpoint::ip_endpoint(const sockaddr_in &ipv4_sock) noexcept
    : _sock_len(0), _address(ipv4_sock.sin_addr), _port(ntohs(ipv4_sock.sin_port)) {
  init_native_socket();
}

ip_endpoint::ip_endpoint(const sockaddr_in6 &ipv6_sock) noexcept
    : _sock_len(0), _address(ipv6_sock.sin6_addr), _port(ntohs(ipv6_sock.sin6_port)) {
  init_native_socket();
}

const native_socket_storage &ip_endpoint::native_socket() const noexcept { return _sock; }

socklen_t ip_endpoint::native_socket_length() const noexcept { return _sock_len; }

const ip_address &ip_endpoint::address() const noexcept { return _address; }

uint16_t ip_endpoint::port() const noexcept { return _port; }

std::string ip_endpoint::to_string() const noexcept {
  std::stringstream ss;
  ss << _address.to_string() << ":" << _port;
  return ss.str();
}

void ip_endpoint::init_native_socket() noexcept {
  if (_address.family() == AF_INET) {
    sockaddr_in *ipv4 = (sockaddr_in *)(&_sock);
    ipv4->sin_family = AF_INET;
    ipv4->sin_addr = _address.native_address().ipv4();
    ipv4->sin_port = htons(_port);
    std::memset(&(ipv4->sin_zero), 0, 8);
    _sock_len = sizeof(sockaddr_in);
  } else if (_address.family() == AF_INET6) {
    sockaddr_in6 *ipv6 = (sockaddr_in6 *)(&_sock);
    ipv6->sin6_family = AF_INET6;
    ipv6->sin6_addr = _address.native_address().ipv6();
    ipv6->sin6_port = htons(_port);
    ipv6->sin6_flowinfo = 0;
    ipv6->sin6_scope_id = 0;
    _sock_len = sizeof(sockaddr_in6);
  }
}

bool operator==(const ip_endpoint &lhs, const ip_endpoint &rhs) noexcept {
  return lhs.to_string() == rhs.to_string();
}

bool operator!=(const ip_endpoint &lhs, const ip_endpoint &rhs) noexcept { return !(lhs == rhs); }

}  // namespace baba::os