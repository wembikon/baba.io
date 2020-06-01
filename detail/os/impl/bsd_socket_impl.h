/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "os/socket/ip_endpoint.h"
#include "os/socket/socket_option.h"

namespace baba::os::socket_impl {

struct create_socket_result final {
  error_code ec = ec::OK;
  io_handle fd = INVALID_HANDLE_VALUE;
};

create_socket_result create_tcp(int af) noexcept;
create_socket_result create_udp(int af) noexcept;

struct accept_socket_result final {
  error_code ec = ec::OK;
  io_handle peer_fd = INVALID_HANDLE_VALUE;
  ip_endpoint peer_ep;
};

accept_socket_result accept(io_handle acceptor_fd, const ip_endpoint &acceptor_ep) noexcept;

error_code connect(io_handle fd, const ip_endpoint &peer_ep) noexcept;

struct socket_result final {
  error_code ec = ec::OK;
  int bytes = 0;
};

socket_result send(io_handle fd, const uint8_t *buffer, int size) noexcept;
socket_result recv(io_handle fd, uint8_t *buffer, int size) noexcept;
socket_result send_to(io_handle fd, const uint8_t *buffer, int size,
                      const ip_endpoint &peer_ep) noexcept;

struct recv_from_socket_result final {
  error_code ec = ec::OK;
  int bytes = 0;
  ip_endpoint peer_ep;
};

recv_from_socket_result recv_from(io_handle fd, uint8_t *buffer, int size,
                                  const ip_endpoint &receiver_ep) noexcept;

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