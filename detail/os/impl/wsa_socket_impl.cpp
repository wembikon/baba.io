/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/wsa_socket_impl.h"

#include <system_error>  // errc

namespace baba::os::socket_impl {

create_socket_result create_tcp(int af) noexcept {
  if (const auto fd = ::socket(af, SOCK_STREAM, 0); fd == INVALID_SOCKET) {
    return {WSAGetLastError(), (HANDLE)INVALID_SOCKET};
  } else {
    return {ec::OK, (HANDLE)fd};
  }
}

create_socket_result create_udp(int af) noexcept {
  if (const auto fd = ::socket(af, SOCK_DGRAM, 0); fd == INVALID_SOCKET) {
    return {WSAGetLastError(), (HANDLE)INVALID_SOCKET};
  } else {
    return {ec::OK, (HANDLE)fd};
  }
}

get_acceptex_result get_acceptex_fn(io_handle acceptor_fd) noexcept {
  GUID acceptex_guid = WSAID_ACCEPTEX;
  LPFN_ACCEPTEX lpfnAcceptEx = NULL;
  DWORD bytes_returned = 0;
  if (WSAIoctl((SOCKET)acceptor_fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &acceptex_guid,
               sizeof(acceptex_guid), &lpfnAcceptEx, sizeof(lpfnAcceptEx), &bytes_returned, NULL,
               NULL) == SOCKET_ERROR) {
    return {WSAGetLastError(), lpfnAcceptEx};
  }
  return {ec::OK, lpfnAcceptEx};
}

pre_accept_socket_result pre_accept(io_handle acceptor_fd, const ip_endpoint &acceptor_ep,
                                    LPFN_ACCEPTEX acceptex_fn, uint8_t *ep_buffer,
                                    LPOVERLAPPED evt) noexcept {
  const auto peer_fd = ::socket(acceptor_ep.address().family(), SOCK_STREAM, 0);
  if (peer_fd == INVALID_SOCKET) {
    return {(error_code)std::errc::bad_file_descriptor, (HANDLE)INVALID_SOCKET};
  } else {
    DWORD ep_size = acceptor_ep.native_socket_length() + 16, dummy;
    if (acceptex_fn((SOCKET)acceptor_fd, (SOCKET)peer_fd, ep_buffer, 0, ep_size, ep_size, &dummy,
                    evt) != TRUE) {
      if (GetLastError() != ERROR_IO_PENDING) {
        return {(error_code)GetLastError(), (HANDLE)INVALID_SOCKET};
      }
    }
  }
  return {ec::OK, (HANDLE)peer_fd};
}

post_accept_socket_result post_accept(io_handle acceptor_fd, const ip_endpoint &acceptor_ep,
                                      io_handle peer_fd, LPOVERLAPPED evt) noexcept {
  DWORD b = 0, f = 0;  // Ignore bytes receive and flags
  if (!WSAGetOverlappedResult((SOCKET)acceptor_fd, evt, &b, true, &f)) {
    return {WSAGetLastError(), {}};
  } else {
    if (::setsockopt((SOCKET)peer_fd, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&acceptor_fd,
                     sizeof(acceptor_fd)) == -1) {
      return {WSAGetLastError(), {}};
    } else {
      if (acceptor_ep.address().family() == family::v4()) {
        sockaddr_in sock;
        socklen_t len = sizeof(sockaddr_in);
        if (::getpeername((SOCKET)peer_fd, (sockaddr *)&sock, &len) == -1) {
          return {WSAGetLastError(), {}};
        } else {
          return {ec::OK, ip_endpoint(sock)};
        }
      } else {
        sockaddr_in6 sock;
        socklen_t len = sizeof(sockaddr_in6);
        if (::getpeername((SOCKET)peer_fd, (sockaddr *)&sock, &len) != 0) {
          return {WSAGetLastError(), {}};
        } else {
          return {ec::OK, ip_endpoint(sock)};
        }
      }
    }
  }
}

get_connectex_result get_connectex_fn(io_handle connector_fd) noexcept {
  GUID connectex_guid = WSAID_CONNECTEX;
  LPFN_CONNECTEX lpfnConnectEx = NULL;
  DWORD bytes_returned = 0;
  if (WSAIoctl((SOCKET)connector_fd, SIO_GET_EXTENSION_FUNCTION_POINTER, &connectex_guid,
               sizeof(connectex_guid), &lpfnConnectEx, sizeof(lpfnConnectEx), &bytes_returned, NULL,
               NULL) != 0) {
    return {WSAGetLastError(), lpfnConnectEx};
  }
  return {ec::OK, lpfnConnectEx};
}

error_code pre_connect(io_handle connector_fd, const ip_endpoint &connector_ep,
                       LPFN_CONNECTEX connectex_fn, LPOVERLAPPED evt) noexcept {
  socklen_t sock_len = connector_ep.native_socket_length();
  if (connector_ep.address().family() == family::v4()) {
    DWORD dummy;
    const sockaddr_in &sock = connector_ep.native_socket().ipv4();
    if (connectex_fn((SOCKET)connector_fd, (sockaddr *)&sock, sock_len, NULL, 0, &dummy, evt) !=
        TRUE) {
      const auto e = WSAGetLastError();
      if (e != ERROR_IO_PENDING) {
        return e;
      }
    }
  } else {
    DWORD dummy;
    const sockaddr_in6 &sock = connector_ep.native_socket().ipv6();
    if (connectex_fn((SOCKET)connector_fd, (sockaddr *)&sock, sock_len, NULL, 0, &dummy, evt) !=
        TRUE) {
      const auto e = WSAGetLastError();
      if (e != ERROR_IO_PENDING) {
        return e;
      }
    }
  }
  return ec::OK;
}

error_code post_connect(io_handle connector_fd, LPOVERLAPPED evt) noexcept {
  DWORD b = 0, f = 0;  // ignore bytes sent and flags
  if (!WSAGetOverlappedResult((SOCKET)connector_fd, evt, &b, true, &f)) {
    return WSAGetLastError();
  } else {
    if (::setsockopt((SOCKET)connector_fd, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT,
                     (char *)&connector_fd, sizeof(connector_fd)) == -1) {
      return WSAGetLastError();
    }
  }
  return ec::OK;
}

error_code pre_send(io_handle fd, WSABUF &buffer, LPOVERLAPPED evt) noexcept {
  if (WSASend((SOCKET)fd, &buffer, 1, NULL, 0, evt, NULL) != 0) {
    const auto e = WSAGetLastError();
    if (e != WSA_IO_PENDING) {
      return e;
    }
  }
  return ec::OK;
}

socket_result post_send(io_handle fd, LPOVERLAPPED evt) noexcept {
  DWORD f = 0;  // ignore flags
  int bytes_sent = 0;
  if (!WSAGetOverlappedResult((SOCKET)fd, evt, (LPDWORD)&bytes_sent, true, &f)) {
    return {WSAGetLastError(), -1};
  }
  return {ec::OK, bytes_sent};
}

error_code pre_recv(io_handle fd, WSABUF &buffer, LPOVERLAPPED evt) noexcept {
  DWORD f = 0;  // ignore flags
  if (WSARecv((SOCKET)fd, &buffer, 1, NULL, &f, evt, NULL) != 0) {
    const auto e = WSAGetLastError();
    if (e != WSA_IO_PENDING) {
      return e;
    }
  }
  return ec::OK;
}

socket_result post_recv(io_handle fd, LPOVERLAPPED evt) noexcept {
  DWORD f = 0;  // ignore flags
  int bytes_recv = 0;
  if (!WSAGetOverlappedResult((SOCKET)fd, evt, (LPDWORD)&bytes_recv, true, &f)) {
    return {WSAGetLastError(), -1};
  }
  return {ec::OK, bytes_recv};
}

error_code pre_send_to(io_handle fd, WSABUF &buffer, const ip_endpoint &peer_ep,
                       LPOVERLAPPED evt) noexcept {
  socklen_t sock_len = peer_ep.native_socket_length();
  if (peer_ep.address().family() == family::v4()) {
    const sockaddr_in &sock = peer_ep.native_socket().ipv4();
    if (WSASendTo((SOCKET)fd, &buffer, 1, NULL, 0, (const struct sockaddr *)&sock, sock_len, evt,
                  NULL) != 0) {
      const auto e = WSAGetLastError();
      if (e != WSA_IO_PENDING) {
        return e;
      }
    }
  } else {
    const sockaddr_in6 &sock = peer_ep.native_socket().ipv6();
    if (WSASendTo((SOCKET)fd, &buffer, 1, NULL, 0, (const struct sockaddr *)&sock, sock_len, evt,
                  NULL) != 0) {
      const auto e = WSAGetLastError();
      if (e != WSA_IO_PENDING) {
        return e;
      }
    }
  }
  return ec::OK;
}

socket_result post_send_to(io_handle fd, LPOVERLAPPED evt) noexcept {
  DWORD f = 0;  // ignore flags
  int bytes_sent = 0;
  if (!WSAGetOverlappedResult((SOCKET)fd, evt, (LPDWORD)&bytes_sent, true, &f)) {
    return {WSAGetLastError(), -1};
  }
  return {ec::OK, bytes_sent};
}

error_code pre_recv_from(io_handle fd, WSABUF &buffer, sockaddr_storage &peer_sock,
                         socklen_t &sock_len, LPOVERLAPPED evt) noexcept {
  DWORD f = 0;  // ignore flags
  if (WSARecvFrom((SOCKET)fd, &buffer, 1, NULL, &f, (struct sockaddr *)(&peer_sock), &sock_len, evt,
                  NULL) != 0) {
    const auto e = WSAGetLastError();
    if (e != WSA_IO_PENDING) {
      return e;
    }
  }
  return ec::OK;
}

recv_from_socket_result post_recv_from(io_handle fd, const ip_endpoint &receiver_ep,
                                       sockaddr_storage &peer_sock, LPOVERLAPPED evt) noexcept {
  DWORD f = 0;  // ignore flags
  int bytes_recv = 0;
  if (!WSAGetOverlappedResult((SOCKET)fd, evt, (LPDWORD)&bytes_recv, true, &f)) {
    return {WSAGetLastError(), -1, {}};
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
  if (::shutdown((SOCKET)fd, how) == -1) {
    return WSAGetLastError();
  }
  return ec::OK;
}

error_code set_option(io_handle fd, const int_option &opt) noexcept {
  int val = opt.get_value();
  if (::setsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (const char *)&val, sizeof(int)) ==
      -1) {
    return WSAGetLastError();
  }
  return ec::OK;
}

error_code set_option(io_handle fd, const address_option &opt) noexcept {
  socklen_t addr_len = opt.get_address().native_address_length();
  if (opt.get_address().family() == family::v4()) {
    const in_addr &addr = opt.get_address().native_address().ipv4();
    if (::setsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (const char *)&(addr),
                     addr_len) == -1) {
      return WSAGetLastError();
    }
  } else if (opt.get_address().family() == family::v6()) {
    const in6_addr &addr = opt.get_address().native_address().ipv6();
    if (::setsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (const char *)&(addr),
                     addr_len) == -1) {
      return WSAGetLastError();
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
    if (::setsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (const char *)&group,
                     sizeof(struct ip_mreq)) == -1) {
      return WSAGetLastError();
    }
  } else if (opt.get_iface_address().family() == family::v6()) {
    const in6_addr &mcast_addr = opt.get_mcast_address().native_address().ipv6();
    struct ipv6_mreq group;
    group.ipv6mr_interface = opt.get_iface_index();
    std::memcpy(group.ipv6mr_multiaddr.s6_addr, mcast_addr.s6_addr,
                sizeof(group.ipv6mr_multiaddr.s6_addr));
    if (::setsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (const char *)&group,
                     sizeof(struct ipv6_mreq)) == -1) {
      return WSAGetLastError();
    }
  }
  return ec::OK;
}

error_code get_option(io_handle fd, int_option &opt) noexcept {
  int optval = 0;
  socklen_t optval_size = sizeof(int);
  if (::getsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (char *)&optval, &optval_size) ==
      -1) {
    return WSAGetLastError();
  }
  opt.set_value(optval);
  return ec::OK;
}

error_code get_option(io_handle fd, address_option &opt) noexcept {
  if (opt.get_address().family() == family::v4()) {
    struct in_addr iface;
    socklen_t ifacesize = sizeof(struct in_addr);
    if (::getsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (char *)&iface, &ifacesize) ==
        -1) {
      return WSAGetLastError();
    }
    opt.set_address(ip_address(iface));
  } else if (opt.get_address().family() == family::v6()) {
    struct in6_addr iface;
    socklen_t ifacesize = sizeof(struct in6_addr);
    if (::getsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (char *)&iface, &ifacesize) ==
        -1) {
      return WSAGetLastError();
    }
    opt.set_address(ip_address(iface));
  }
  return ec::OK;
}

error_code get_option(io_handle fd, group_option &opt) noexcept {
  if (opt.get_iface_address().family() == family::v4()) {
    struct ip_mreq group;
    socklen_t groupsize = sizeof(group);
    if (::getsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (char *)&group, &groupsize) ==
        -1) {
      return WSAGetLastError();
    }
    opt.set_iface_address(ip_address(group.imr_interface));
    opt.set_mcast_address(ip_address(group.imr_multiaddr));
  } else if (opt.get_iface_address().family() == family::v6()) {
    struct ipv6_mreq group;
    socklen_t groupsize = sizeof(group);
    if (::getsockopt((SOCKET)fd, opt.get_level(), opt.get_name(), (char *)&group, &groupsize) ==
        -1) {
      return WSAGetLastError();
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
    if (::bind((SOCKET)fd, (const struct sockaddr *)&sock, sock_len) == -1) {
      return WSAGetLastError();
    }
  } else if (local_ep.address().family() == family::v6()) {
    const sockaddr_in6 &sock = local_ep.native_socket().ipv6();
    if (::bind((SOCKET)fd, (const struct sockaddr *)&sock, sock_len) == -1) {
      return WSAGetLastError();
    }
  }
  return ec::OK;
}

error_code listen(io_handle fd, int backlog) noexcept {
  if (::listen((SOCKET)fd, backlog) == -1) {
    return WSAGetLastError();
  }
  return ec::OK;
}

}  // namespace baba::os::socket_impl