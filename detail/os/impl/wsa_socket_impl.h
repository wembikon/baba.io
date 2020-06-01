/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/io_types.h"
#include "os/socket/ip_endpoint.h"
#include "os/socket/socket_option.h"

namespace baba::os::socket_impl {

struct create_socket_result final {
  error_code ec = ec::OK;
  io_handle fd = INVALID_HANDLE_VALUE;
};

create_socket_result create_tcp(int af) noexcept;
create_socket_result create_udp(int af) noexcept;

struct get_acceptex_result final {
  error_code ec = ec::OK;
  LPFN_ACCEPTEX acceptex_fn;
};

get_acceptex_result get_acceptex_fn(io_handle acceptor_fd) noexcept;

struct pre_accept_socket_result final {
  error_code ec = ec::OK;
  io_handle peer_fd = INVALID_HANDLE_VALUE;
};

pre_accept_socket_result pre_accept(io_handle acceptor_fd, const ip_endpoint &acceptor_ep,
                                    LPFN_ACCEPTEX acceptex_fn, uint8_t *ep_buffer,
                                    LPOVERLAPPED evt) noexcept;

struct post_accept_socket_result final {
  error_code ec = ec::OK;
  ip_endpoint peer_ep;
};

post_accept_socket_result post_accept(io_handle acceptor_fd, const ip_endpoint &acceptor_ep,
                                      io_handle peer_fd, LPOVERLAPPED evt) noexcept;

struct get_connectex_result final {
  error_code ec = ec::OK;
  LPFN_CONNECTEX connectex_fn;
};

get_connectex_result get_connectex_fn(io_handle connector_fd) noexcept;

error_code pre_connect(io_handle connector_fd, const ip_endpoint &connector_ep,
                       LPFN_CONNECTEX connectex_fn, LPOVERLAPPED evt) noexcept;

error_code post_connect(io_handle connector_fd, LPOVERLAPPED evt) noexcept;

struct socket_result final {
  error_code ec = ec::OK;
  int bytes = 0;
};

error_code pre_send(io_handle fd, WSABUF &buffer, LPOVERLAPPED evt) noexcept;
socket_result post_send(io_handle fd, LPOVERLAPPED evt) noexcept;

error_code pre_recv(io_handle fd, WSABUF &buffer, LPOVERLAPPED evt) noexcept;
socket_result post_recv(io_handle fd, LPOVERLAPPED evt) noexcept;

error_code pre_send_to(io_handle fd, WSABUF &buffer, LPOVERLAPPED evt) noexcept;
socket_result post_send_to(io_handle fd, const ip_endpoint &peer_ep, LPOVERLAPPED evt) noexcept;

struct recv_from_socket_result final {
  error_code ec = ec::OK;
  int bytes = 0;
  ip_endpoint peer_ep;
};

error_code pre_recv_from(io_handle fd, WSABUF &buffer, sockaddr_storage &sock, socklen_t &sock_len,
                         LPOVERLAPPED evt) noexcept;
recv_from_socket_result post_recv_from(io_handle fd, const ip_endpoint &receiver_ep,
                                       sockaddr_storage &peer_sock, LPOVERLAPPED evt) noexcept;

error_code shutdown(io_handle fd, int how) noexcept;
error_code set_option(io_handle fd, const int_option &opt) noexcept;
error_code set_option(io_handle fd, const address_option &opt) noexcept;
error_code set_option(io_handle fd, const group_option &opt) noexcept;
error_code get_option(io_handle fd, int_option &opt) noexcept;
error_code get_option(io_handle fd, address_option &opt) noexcept;
error_code get_option(io_handle fd, group_option &opt) noexcept;
error_code bind(io_handle fd, const ip_endpoint &local_ep) noexcept;
error_code listen(io_handle fd, int backlog) noexcept;

}  // namespace baba::os::socket_impl