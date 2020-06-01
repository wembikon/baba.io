/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "tcp_connector_pimpl.h"

#include "ip_endpoint_pimpl.h"

namespace baba {

tcp_connector::tcp_connector(tcp_connector &&tmp) noexcept : _pimpl(new pimpl(std::move(tmp))) {}

tcp_connector &tcp_connector::operator=(tcp_connector &&tmp) noexcept {
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

tcp_connector::tcp_connector() noexcept : _pimpl(new pimpl()) {}

tcp_connector::~tcp_connector() noexcept = default;

tcp_connector::tcp_connector(io_strand &strand, address_family af) noexcept
    : _strand(&strand), _pimpl(new pimpl(strand, af)) {}

void tcp_connector::connect(const ip_endpoint &ep, const on_connect &cb) noexcept {
  _pimpl->impl.connect(ep._pimpl->impl, cb);
}

io_strand &tcp_connector::strand() noexcept {
  RUNTIME_ASSERT(_strand);
  return *_strand;
}

io_handle tcp_connector::fd() const noexcept { return _pimpl->impl.fd(); }

ip_endpoint tcp_connector::endpoint() const noexcept {
  ip_endpoint ep;
  ep._pimpl->impl = _pimpl->impl.endpoint();
  return ep;
}

}  // namespace baba