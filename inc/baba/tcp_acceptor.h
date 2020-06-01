/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/address_family.h"
#include "baba/error_code.h"
#include "baba/io_handle.h"
#include "baba/io_strand.h"
#include "baba/ip_endpoint.h"
#include "baba/semantics.h"
#include "baba/tcp_socket.h"

#include <functional>
#include <memory>

namespace baba {

using on_accept = std::function<void(error_code)>;

class tcp_acceptor final {
 public:
  NOT_COPYABLE(tcp_acceptor)

  struct pimpl;

  tcp_acceptor(tcp_acceptor &&tmp) noexcept;
  tcp_acceptor &operator=(tcp_acceptor &&tmp) noexcept;
  tcp_acceptor() noexcept;
  ~tcp_acceptor() noexcept;

  tcp_acceptor(io_strand &strand, address_family af) noexcept;
  error_code bind(const ip_endpoint &ep) noexcept;
  error_code listen(int backlog) noexcept;
  void accept(tcp_socket &peer, const on_accept &cb) noexcept;
  void accept(io_strand &strand, tcp_socket &peer, const on_accept &cb) noexcept;
  io_strand &strand() noexcept;
  io_handle fd() const noexcept;
  ip_endpoint endpoint() const noexcept;

 private:
  friend class tcp_socket;
  io_strand *_strand = nullptr;
  std::unique_ptr<pimpl> _pimpl;
};

}  // namespace baba