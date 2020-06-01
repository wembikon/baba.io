/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/bsd_socket_impl.h"

#include "baba/io_types.h"

#include <cstring>  // memcpy

namespace baba::os::socket_impl {

create_socket_result create_tcp(int af) noexcept {
  if (const auto fd = ::socket(af, SOCK_STREAM, 0); fd == INVALID_SOCKET) {
    return {errno, INVALID_SOCKET};
  } else {
    return {ec::OK, fd};
  }
}

create_socket_result create_udp(int af) noexcept {
  if (const auto fd = ::socket(af, SOCK_DGRAM, 0); fd == INVALID_SOCKET) {
    return {errno, INVALID_SOCKET};
  } else {
    return {ec::OK, fd};
  }
}

accept_socket_result accept(io_handle acceptor_fd, const ip_endpoint &acceptor_ep) noexcept {
  sockaddr_storage peer_sock;
  socklen_t sock_len = acceptor_ep.native_socket_length();
  const int peer_fd = ::accept(acceptor_fd, (struct sockaddr *)&peer_sock, &sock_len);
  if (peer_fd == INVALID_SOCKET) {
    return {errno, INVALID_SOCKET, {}};
  } else {
    // Create endpoint based on the acceptors endpoint address family
    if (acceptor_ep.address().family() == family::v4()) {
      sockaddr_in *sock = (sockaddr_in *)&peer_sock;
      return {ec::OK, peer_fd, ip_endpoint(*sock)};
    } else {
      sockaddr_in6 *sock = (sockaddr_in6 *)&peer_sock;
      return {ec::OK, peer_fd, ip_endpoint(*sock)};
    }
  }
}

error_code connect(io_handle fd, const ip_endpoint &peer_ep) noexcept {
  socklen_t sock_len = peer_ep.native_socket_length();
  if (peer_ep.address().family() == family::v4()) {
    const sockaddr_in &sock = peer_ep.native_socket().ipv4();
    if (::connect(fd, (const struct sockaddr *)&sock, sock_len) == -1) {
      if (errno != EINPROGRESS) {
        return errno;
      }
    }
  } else if (peer_ep.address().family() == family::v4()) {
    const sockaddr_in6 &sock = peer_ep.native_socket().ipv6();
    if (::connect(fd, (const struct sockaddr *)&sock, sock_len) == -1) {
      if (errno != EINPROGRESS) {
        return errno;
      }
    }
  }
  return ec::OK;
}

socket_result send(io_handle fd, const uint8_t *buffer, int size) noexcept {
  const int bytes_sent = ::send(fd, (const char *)buffer, static_cast<int>(size), 0);
  if (bytes_sent == -1) {
    return {errno, -1};
  }
  return {ec::OK, bytes_sent};
}

socket_result recv(io_handle fd, uint8_t *buffer, int size) noexcept {
  const int bytes_recv = ::recv(fd, (char *)buffer, static_cast<int>(size), 0);
  if (bytes_recv == -1) {
    return {errno, -1};
  }
  return {ec::OK, bytes_recv};
}

socket_result send_to(io_handle fd, const uint8_t *buffer, int size,
                      const ip_endpoint &peer_ep) noexcept {
  socklen_t sock_len = peer_ep.native_socket_length();
  int bytes_sent = 0;
  if (peer_ep.address().family() == family::v4()) {
    const struct sockaddr_in &sock = peer_ep.native_socket().ipv4();
    bytes_sent =
        ::sendto(fd, (const char *)buffer, size, 0, (const struct sockaddr *)&sock, sock_len);
  } else {
    const struct sockaddr_in6 &sock = peer_ep.native_socket().ipv6();
    bytes_sent =
        ::sendto(fd, (const char *)buffer, size, 0, (const struct sockaddr *)&sock, sock_len);
  }
  if (bytes_sent == -1) {
    return {errno, -1};
  }
  return {ec::OK, bytes_sent};
}

recv_from_socket_result recv_from(io_handle fd, uint8_t *buffer, int size,
                                  const ip_endpoint &receiver_ep) noexcept {
  struct sockaddr_storage peer_sock;
  socklen_t peer_sock_len = receiver_ep.native_socket_length();
  int bytes_recv =
      ::recvfrom(fd, (char *)buffer, size, 0, (struct sockaddr *)(&peer_sock), &peer_sock_len);
  if (bytes_recv == -1) {
    return {errno, -1, {}};
  } else {
    // Create endpoint based on the receivers endpoint address family
    if (receiver_ep.address().family() == family::v4()) {
      struct sockaddr_in *sock = (struct sockaddr_in *)&peer_sock;
      return {ec::OK, bytes_recv, ip_endpoint(*sock)};
    } else {
      struct sockaddr_in6 *sock = (struct sockaddr_in6 *)&peer_sock;
      return {ec::OK, bytes_recv, ip_endpoint(*sock)};
    }
  }
}

error_code shutdown(io_handle fd, int how) noexcept {
  if (::shutdown(fd, how) == -1) {
    return errno;
  }
  return ec::OK;
}

error_code set_option(io_handle fd, const int_option &opt) noexcept {
  int val = opt.get_value();
  if (::setsockopt(fd, opt.get_level(), opt.get_name(), (const char *)&val, sizeof(int)) == -1) {
    return errno;
  }
  return ec::OK;
}

error_code set_option(io_handle fd, const address_option &opt) noexcept {
  socklen_t addr_len = opt.get_address().native_address_length();
  if (opt.get_address().family() == family::v4()) {
    const in_addr &addr = opt.get_address().native_address().ipv4();
    if (::setsockopt(fd, opt.get_level(), opt.get_name(), (const char *)&(addr), addr_len) == -1) {
      return errno;
    }
  } else if (opt.get_address().family() == family::v6()) {
    const in6_addr &addr = opt.get_address().native_address().ipv6();
    if (::setsockopt(fd, opt.get_level(), opt.get_name(), (const char *)&(addr), addr_len) == -1) {
      return errno;
    }
  }
  return ec::OK;
}

error_code set_option(io_handle fd, const group_option &opt) noexcept {
  if (opt.get_iface_address().family() == family::v4()) {
    const in_addr &iface_addr = opt.get_iface_address().native_address().ipv4();
    const in_addr &mcast_addr = opt.get_mcast_address().native_address().ipv4();
    struct ip_mreq group;
    group.imr_interface.s_addr = iface_addr.s_addr;
    group.imr_multiaddr.s_addr = mcast_addr.s_addr;
    if (::setsockopt(fd, opt.get_level(), opt.get_name(), (const char *)&group,
                     sizeof(struct ip_mreq)) == -1) {
      return errno;
    }
  } else if (opt.get_iface_address().family() == family::v6()) {
    const in6_addr &mcast_addr = opt.get_mcast_address().native_address().ipv6();
    struct ipv6_mreq group;
    group.ipv6mr_interface = opt.get_iface_index();
    std::memcpy(group.ipv6mr_multiaddr.s6_addr, mcast_addr.s6_addr,
                sizeof(group.ipv6mr_multiaddr.s6_addr));
    if (::setsockopt(fd, opt.get_level(), opt.get_name(), (const char *)&group,
                     sizeof(struct ipv6_mreq)) == -1) {
      return errno;
    }
  }
  return ec::OK;
}

error_code get_option(io_handle fd, int_option &opt) noexcept {
  int optval = 0;
  socklen_t optval_size = sizeof(int);
  if (::getsockopt(fd, opt.get_level(), opt.get_name(), (char *)&optval, &optval_size) == -1) {
    return errno;
  }
  opt.set_value(optval);
  return ec::OK;
}

error_code get_option(io_handle fd, address_option &opt) noexcept {
  if (opt.get_address().family() == family::v4()) {
    struct in_addr iface;
    socklen_t ifacesize = sizeof(struct in_addr);
    if (::getsockopt(fd, opt.get_level(), opt.get_name(), (char *)&iface, &ifacesize) == -1) {
      return errno;
    }
    opt.set_address(ip_address(iface));
  } else if (opt.get_address().family() == family::v6()) {
    struct in6_addr iface;
    socklen_t ifacesize = sizeof(struct in6_addr);
    if (::getsockopt(fd, opt.get_level(), opt.get_name(), (char *)&iface, &ifacesize) == -1) {
      return errno;
    }
    opt.set_address(ip_address(iface));
  }
  return ec::OK;
}

error_code get_option(io_handle fd, group_option &opt) noexcept {
  if (opt.get_iface_address().family() == family::v4()) {
    struct ip_mreq group;
    socklen_t groupsize = sizeof(group);
    if (::getsockopt(fd, opt.get_level(), opt.get_name(), (char *)&group, &groupsize) == -1) {
      return errno;
    }
    opt.set_iface_address(ip_address(group.imr_interface));
    opt.set_mcast_address(ip_address(group.imr_multiaddr));
  } else if (opt.get_iface_address().family() == family::v6()) {
    struct ipv6_mreq group;
    socklen_t groupsize = sizeof(group);
    if (::getsockopt(fd, opt.get_level(), opt.get_name(), (char *)&group, &groupsize) == -1) {
      return errno;
    }
    opt.set_iface_address(ip_address("::"));  // Unused?
    opt.set_iface_index(group.ipv6mr_interface);
    opt.set_mcast_address(ip_address(group.ipv6mr_multiaddr));
  }
  return ec::OK;
}

error_code bind(io_handle fd, const ip_endpoint &local_ep) noexcept {
  socklen_t sock_len = local_ep.native_socket_length();
  if (local_ep.address().family() == family::v4()) {
    const sockaddr_in &sock = local_ep.native_socket().ipv4();
    if (::bind(fd, (const struct sockaddr *)&sock, sock_len) == -1) {
      return errno;
    }
  } else if (local_ep.address().family() == family::v6()) {
    const sockaddr_in6 &sock = local_ep.native_socket().ipv6();
    if (::bind(fd, (const struct sockaddr *)&sock, sock_len) == -1) {
      return errno;
    }
  }
  return ec::OK;
}

error_code listen(io_handle fd, int backlog) noexcept {
  if (::listen(fd, backlog) == -1) {
    return errno;
  }
  return ec::OK;
}

}  // namespace baba::os::socket_impl