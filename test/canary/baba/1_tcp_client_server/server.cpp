/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "baba/io_service.h"
#include "baba/ip_endpoint.h"
#include "baba/socket_option.h"
#include "baba/tcp_acceptor.h"
#include "baba/tcp_socket.h"

#include "signal_handler.h"

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using namespace baba;

class peer_socket final : public std::enable_shared_from_this<peer_socket> {
 public:
  peer_socket(tcp_socket &&sock) noexcept : _socket(std::move(sock)) {}
  void start() noexcept { do_recv(); }

 private:
  void do_send() noexcept {
    _msg = "Hello from server!";
    _socket.send((const uint8_t *)_msg.data(), _msg.length(),
                 [self = shared_from_this()](error_code e, int bytes_sent) {
                   if (e == ec::OK) {
                     std::cerr << "Sent " << bytes_sent << " bytes" << std::endl;
                     self->do_recv();
                   } else {
                     std::cerr << "Failed sending. ec=" << e << std::endl;
                   }
                 });
  }
  void do_recv() noexcept {
    _socket.recv((uint8_t *)_recv_buffer, sizeof(_recv_buffer),
                 [self = shared_from_this()](error_code e, int bytes_recv) {
                   if (e == ec::OK) {
                     std::string msg(self->_recv_buffer, bytes_recv);
                     std::cerr << "Received " << bytes_recv << " bytes. Message: " << msg
                               << std::endl;
                     self->do_send();
                   } else {
                     std::cerr << "Failed receiving. ec=" << e << std::endl;
                   }
                 });
  }
  tcp_socket _socket;
  std::string _msg;
  char _recv_buffer[100];
};

class passive_socket final {
 public:
  passive_socket(const std::vector<io_strand *> &strands, const ip_endpoint &acceptor_ep) noexcept
      : _acceptor(*strands[0], acceptor_ep.address().family()),
        _acceptor_ep(acceptor_ep),
        _strands(strands) {}
  void prepare() noexcept {
    if (const auto e = socket::set_option(_acceptor.fd(), reuse_address(true)); e != ec::OK) {
      std::cerr << "Failed to set option. ec=" << e << std::endl;
    }
    if (const auto e = _acceptor.bind(_acceptor_ep); e != ec::OK) {
      std::cerr << "Failed to bind. ec=" << e << std::endl;
    }
    if (const auto e = _acceptor.listen(10); e != ec::OK) {
      std::cerr << "Failed to listen. ec=" << e << std::endl;
    }
  }
  void start() noexcept { do_accept(); }

 private:
  void do_accept() noexcept {
    _acceptor.accept(*_strands[_count++ % _strands.size()], _peer, [this](error_code e) {
      if (e == ec::OK) {
        std::cerr << "Accepted" << std::endl;
        std::make_shared<peer_socket>(std::move(_peer))->start();
        do_accept();
      } else {
        std::cerr << "Failed accept. ec=" << e << std::endl;
      }
    });
  }
  tcp_acceptor _acceptor;
  ip_endpoint _acceptor_ep;
  tcp_socket _peer;
  std::vector<io_strand *> _strands;
  int _count = 0;
};

int main() {
  std::cout << "-- program start --" << std::endl;
  io_service service;
  io_strand strand1(service);
  io_strand strand2(service);
  io_strand strand3(service);
  io_strand strand4(service);
  std::vector<io_strand *> strands{&strand1, &strand2, &strand3, &strand4};

  utils::signal_mgr::add_action(SIGINT, [&service]() {  // ctrl-c
    service.stop();
  });
  std::thread t([&service]() { service.run(); });

  passive_socket sock(strands, ip_endpoint("127.0.0.1", 5001));
  sock.prepare();
  sock.start();

  t.join();
  std::cout << "-- program end --" << std::endl;
  return 0;
}