/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "tcp_acceptor_pimpl.h"

#include "ip_endpoint_pimpl.h"
#include "tcp_socket_pimpl.h"

namespace baba {

tcp_acceptor::tcp_acceptor(tcp_acceptor &&tmp) noexcept : _pimpl(new pimpl(std::move(tmp))) {}

tcp_acceptor &tcp_acceptor::operator=(tcp_acceptor &&tmp) noexcept {
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

tcp_acceptor::tcp_acceptor() noexcept : _pimpl(new pimpl()) {}

tcp_acceptor::~tcp_acceptor() noexcept = default;

tcp_acceptor::tcp_acceptor(io_strand &strand, address_family af) noexcept
    : _strand(&strand), _pimpl(new pimpl(strand, af)) {}

error_code tcp_acceptor::bind(const ip_endpoint &ep) noexcept {
  return _pimpl->impl.bind(ep._pimpl->impl);
}

error_code tcp_acceptor::listen(int backlog) noexcept { return _pimpl->impl.listen(backlog); }

void tcp_acceptor::accept(tcp_socket &peer, const on_accept &cb) noexcept {
  peer._strand = _strand;
  _pimpl->impl.accept(peer._pimpl->impl, cb);
}

void tcp_acceptor::accept(io_strand &strand, tcp_socket &peer, const on_accept &cb) noexcept {
  peer._strand = &strand;
  _pimpl->impl.accept(strand._pimpl->impl, peer._pimpl->impl, cb);
}

io_strand &tcp_acceptor::strand() noexcept {
  RUNTIME_ASSERT(_strand);
  return *_strand;
}

io_handle tcp_acceptor::fd() const noexcept { return _pimpl->impl.fd(); }

ip_endpoint tcp_acceptor::endpoint() const noexcept {
  ip_endpoint ep;
  ep._pimpl->impl = _pimpl->impl.endpoint();
  return ep;
}

}  // namespace baba