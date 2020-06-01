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

#include <functional>
#include <memory>

namespace baba {

using on_connect = std::function<void(error_code)>;

class tcp_connector final {
 public:
  NOT_COPYABLE(tcp_connector)

  struct pimpl;

  tcp_connector(tcp_connector &&tmp) noexcept;
  tcp_connector &operator=(tcp_connector &&tmp) noexcept;
  tcp_connector() noexcept;
  ~tcp_connector() noexcept;

  tcp_connector(io_strand &strand, address_family af) noexcept;
  void connect(const ip_endpoint &ep, const on_connect &cb) noexcept;
  io_strand &strand() noexcept;
  io_handle fd() const noexcept;
  ip_endpoint endpoint() const noexcept;

 private:
  friend class tcp_socket;
  io_strand *_strand = nullptr;
  std::unique_ptr<pimpl> _pimpl;
};

}  // namespace baba