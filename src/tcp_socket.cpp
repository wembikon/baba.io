/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "tcp_socket_pimpl.h"

namespace baba {

tcp_socket::tcp_socket(tcp_socket &&tmp) noexcept : _pimpl(new pimpl(std::move(tmp))) {}

tcp_socket &tcp_socket::operator=(tcp_socket &&tmp) noexcept {
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

tcp_socket::tcp_socket() noexcept : _pimpl(new pimpl()) {}

tcp_socket::~tcp_socket() noexcept = default;

tcp_socket::tcp_socket(io_strand &strand, io_handle fd, const ip_endpoint &ep) noexcept
    : _strand(&strand), _pimpl(new pimpl(strand, fd, ep)) {}

tcp_socket::tcp_socket(tcp_connector &&connector) noexcept
    : _strand(connector._strand), _pimpl(new pimpl(std::move(connector))) {}

tcp_socket &tcp_socket::operator=(tcp_connector &&connector) noexcept {
  _strand = connector._strand;
  _pimpl->impl = std::move(connector._pimpl->impl);
  return *this;
}

void tcp_socket::recv(uint8_t *buffer, int size, const on_recv &cb) noexcept {
  _pimpl->impl.recv(buffer, size, cb);
}

void tcp_socket::send(const uint8_t *buffer, int size, const on_send &cb) noexcept {
  _pimpl->impl.send(buffer, size, cb);
}

io_strand &tcp_socket::strand() noexcept {
  RUNTIME_ASSERT(_strand);
  return *_strand;
}

io_handle tcp_socket::fd() const noexcept { return _pimpl->impl.fd(); }

ip_endpoint tcp_socket::endpoint() const noexcept {
  ip_endpoint ep;
  ep._pimpl->impl = _pimpl->impl.endpoint();
  return ep;
}

}  // namespace baba