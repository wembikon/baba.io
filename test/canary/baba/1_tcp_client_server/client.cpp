/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "baba/io_service.h"
#include "baba/ip_endpoint.h"
#include "baba/tcp_connector.h"
#include "baba/tcp_socket.h"

#include "signal_handler.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using namespace baba;

int get_completed_request_limit(int max_request_limit) {
  srand(time(NULL));
  return rand() % max_request_limit + 1;  // 1 to max_request_limit
}

class active_socket final : public std::enable_shared_from_this<active_socket> {
 public:
  active_socket(io_strand &strand, const ip_endpoint &peer_ep, int max_request_limit) noexcept
      : _connector(strand, peer_ep.address().family()),
        _peer_ep(peer_ep),
        _completed_request_limit(get_completed_request_limit(max_request_limit)) {
    std::cout << "completed request limit " << _completed_request_limit << std::endl;
  }
  ~active_socket() noexcept {
    std::cout << "ending socket after " << _num_completed_requests << " out of "
              << _completed_request_limit << " completed requests" << std::endl;
  }
  void start() noexcept { do_connect(); }

 private:
  bool should_stop() { return _num_completed_requests >= _completed_request_limit; }
  void on_connect(error_code e) noexcept {
    if (e == ec::OK) {
      std::cerr << "Connected" << std::endl;
      _socket = std::move(_connector);
      _msg = "Hello from client!";
      do_send();
    } else {
      std::cerr << "Failed connecting. ec=" << e << std::endl;
    }
  }
  void do_connect() noexcept {
    _connector.connect(_peer_ep,
                       [self = shared_from_this()](error_code e) { self->on_connect(e); });
  }
  void on_send(error_code e, int bytes_sent) noexcept {
    if (e == ec::OK) {
      std::cerr << "Sent " << bytes_sent << " bytes" << std::endl;
      do_recv();
    } else {
      std::cerr << "Failed sending. ec=" << e << std::endl;
    }
  }
  void do_send() noexcept {
    _socket.send((const uint8_t *)_msg.data(), _msg.length(),
                 [self = shared_from_this()](error_code e, int bytes_sent) {
                   self->on_send(e, bytes_sent);
                 });
  }
  void on_recv(error_code e, int bytes_recv) noexcept {
    ++_num_completed_requests;
    if (e == ec::OK) {
      std::string msg(_recv_buffer, bytes_recv);
      std::cerr << "Received " << bytes_recv << " bytes. Message: " << msg << std::endl;
      if (should_stop()) {
        std::cerr << "Finished" << std::endl;
      } else {
        do_send();
      }
    } else {
      std::cerr << "Failed sending. ec=" << e << std::endl;
    }
  }
  void do_recv() noexcept {
    _socket.recv((uint8_t *)_recv_buffer, sizeof(_recv_buffer),
                 [self = shared_from_this()](error_code e, int bytes_recv) {
                   self->on_recv(e, bytes_recv);
                 });
  }
  tcp_connector _connector;
  tcp_socket _socket;
  ip_endpoint _peer_ep;
  std::string _msg;
  char _recv_buffer[100];

  // Random stoppers
  int _num_completed_requests = 0;
  int _completed_request_limit;
};

using active_socket_ptr = std::shared_ptr<active_socket>;

active_socket_ptr make_active_socket(io_strand &strand, const ip_endpoint &peer_ep,
                                     int max_request_limit) noexcept {
  return active_socket_ptr(new active_socket(strand, peer_ep, max_request_limit));
}

int main() {
  std::cout << "-- program start --" << std::endl;

  constexpr int socket_count = 100;
  constexpr int max_request_limit = 100000;

  std::atomic<bool> end_loop = false;
  io_service service;
  io_strand strand1(service);
  io_strand strand2(service);
  io_strand strand3(service);
  io_strand strand4(service);
  std::vector<io_strand *> strands{&strand1, &strand2, &strand3, &strand4};

  utils::signal_mgr::add_action(SIGINT, [&service, &end_loop]() {
    end_loop.store(true);
    service.stop();
  });
  std::thread t([&service]() { service.run(); });

  for (int i = 0; i < socket_count; ++i) {
    auto strand = strands[i % strands.size()];
    make_active_socket(*strand, ip_endpoint("127.0.0.1", 5001), max_request_limit)->start();
    if (end_loop.load()) {
      break;
    }
  }

  t.join();
  std::cout << "-- program end --" << std::endl;
  return 0;
}