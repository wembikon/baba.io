/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/io_strand.h"
#include "baba/ip_endpoint.h"
#include "baba/semantics.h"
#include "baba/tcp_connector.h"

#include <functional>
#include <memory>

namespace baba {

using on_recv = std::function<void(error_code, int)>;
using on_send = std::function<void(error_code, int)>;

class tcp_socket final {
 public:
  NOT_COPYABLE(tcp_socket)

  struct pimpl;

  tcp_socket(tcp_socket &&tmp) noexcept;
  tcp_socket &operator=(tcp_socket &&tmp) noexcept;
  tcp_socket() noexcept;
  ~tcp_socket() noexcept;

  tcp_socket(io_strand &strand, io_handle fd, const ip_endpoint &ep) noexcept;
  tcp_socket(tcp_connector &&connector) noexcept;
  tcp_socket &operator=(tcp_connector &&connector) noexcept;
  void recv(uint8_t *buffer, int size, const on_recv &cb) noexcept;
  void send(const uint8_t *buffer, int size, const on_send &cb) noexcept;
  io_strand &strand() noexcept;
  io_handle fd() const noexcept;
  ip_endpoint endpoint() const noexcept;

 private:
  friend class tcp_acceptor;
  io_strand *_strand = nullptr;
  std::unique_ptr<pimpl> _pimpl;
};

}  // namespace baba