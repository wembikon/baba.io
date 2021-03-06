/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/impl/bsd_socket_impl.h"
#include "os/impl/epoll_impl.h"
#include "os/impl/unix_file_impl.h"
#include "os/reactor/epoll/epoll_service.h"
#include "os/reactor/epoll/events/epoll_recv.h"
#include "os/reactor/epoll/events/epoll_send.h"

#include "os/reactor/epoll/io/epoll_async_connector.h"

#include <mutex>

namespace baba::os {

class async_tcp final {
 public:
  NOT_COPYABLE(async_tcp)

  async_tcp() = default;

  async_tcp(async_tcp &&tmp) noexcept
      : _strand_for_user(tmp._strand_for_user),
        _strand(tmp._strand),
        _descriptor(std::move(tmp._descriptor)),
        _recver(std::move(tmp._recver)),
        _sender(std::move(tmp._sender)),
        _ep(std::move(tmp._ep)) {
    tmp._strand_for_user = nullptr;
    tmp._strand = nullptr;
  }

  async_tcp &operator=(async_tcp &&tmp) noexcept {
    _strand_for_user = tmp._strand_for_user;
    _strand = tmp._strand;
    _descriptor = std::move(tmp._descriptor);
    _recver = std::move(tmp._recver);
    _sender = std::move(tmp._sender);
    _ep = std::move(tmp._ep);
    tmp._strand_for_user = nullptr;
    tmp._strand = nullptr;
    return *this;
  }

  async_tcp(completion_strand &strand, io_handle fd, const ip_endpoint &ep) noexcept
      : _strand_for_user(&strand),
        _strand(strand.impl()),
        _descriptor(new reactor_io_descriptor()),
        _recver(
            _descriptor, _strand, _strand->service().registrar(),
            _strand->service().deletion_queue(),
            [epoll_fd = _strand->service().reactor().fd()](reactor_event *evt) {
              evt->descriptor->set_read_mask();
              return epoll_impl::arm(epoll_fd, evt->descriptor->fd, evt->descriptor->event_mask,
                                     evt->descriptor.get());
            },
            [](io_handle fd, uint8_t *buffer, int size, int &bytes_recv) {
              const auto [e, bytes] = socket_impl::recv(fd, buffer, size);
              bytes_recv = bytes;
              return e;
            }),
        _sender(
            _descriptor, _strand, _strand->service().registrar(),
            _strand->service().deletion_queue(),
            [epoll_fd = _strand->service().reactor().fd()](reactor_event *evt) {
              evt->descriptor->set_write_mask();
              return epoll_impl::arm(epoll_fd, evt->descriptor->fd, evt->descriptor->event_mask,
                                     evt->descriptor.get());
            },
            [](io_handle fd, const uint8_t *buffer, int size, int &bytes_sent) {
              const auto [e, bytes] = socket_impl::send(fd, buffer, size);
              bytes_sent = bytes;
              return e;
            }),
        _ep(ep) {
    RUNTIME_ASSERT(_strand);
    _descriptor->fd = fd;
  }

  async_tcp(async_connector &&connector) noexcept
      : async_tcp(connector.strand(), connector.fd(), std::move(connector._connector_ep)) {
    connector._descriptor.reset();
    connector._strand = nullptr;
  }

  void operator=(async_connector &&connector) noexcept {
    _strand = connector._strand;
    _descriptor = std::move(connector._descriptor);
    _recver = epoll_recv(
        _descriptor, _strand, _strand->service().registrar(), _strand->service().deletion_queue(),
        [epoll_fd = _strand->service().reactor().fd()](reactor_event *evt) {
          evt->descriptor->set_read_mask();
          return epoll_impl::arm(epoll_fd, evt->descriptor->fd, evt->descriptor->event_mask,
                                 evt->descriptor.get());
        },
        [](io_handle fd, uint8_t *buffer, int size, int &bytes_recv) {
          const auto [e, bytes] = socket_impl::recv(fd, buffer, size);
          bytes_recv = bytes;
          return e;
        });
    _sender = epoll_send(
        _descriptor, _strand, _strand->service().registrar(), _strand->service().deletion_queue(),
        [epoll_fd = _strand->service().reactor().fd()](reactor_event *evt) {
          evt->descriptor->set_write_mask();
          return epoll_impl::arm(epoll_fd, evt->descriptor->fd, evt->descriptor->event_mask,
                                 evt->descriptor.get());
        },
        [](io_handle fd, const uint8_t *buffer, int size, int &bytes_sent) {
          const auto [e, bytes] = socket_impl::send(fd, buffer, size);
          bytes_sent = bytes;
          return e;
        });
    _ep = std::move(connector._connector_ep);
    connector._strand = nullptr;
    RUNTIME_ASSERT(_strand);
  }

  ~async_tcp() noexcept {
    // Check whether it is not default constructed or been moved
    if (_strand) {
      // When running `complete`, it is possible that the callback it execute will call this
      // destructor. For example, if this object is created as a shared_ptr and the callback
      // reset its own object, if this happens, it will be a deadlock since `complete` is waiting
      // for the destructor to complete but can't because they are locking the same mutex. To
      // prevent this deadlock, we use a recursive mutex instead.
      std::lock_guard lock(_descriptor->in_scope_mtx);
      if (_descriptor->in_scope) {
        _descriptor->in_scope = false;
        // Remove event from the reactor. Deletion event will handle its clean up
        epoll_impl::remove(_strand->service().reactor().fd(), _descriptor->fd);
        unix_file_impl::close(_descriptor->fd);
        _recver.dispose();
        _sender.dispose();
      }
    }
  }

  void recv(uint8_t *buffer, int size, const epoll_recv::recved_fn &cb) noexcept {
    RUNTIME_ASSERT(_strand);
    _recver.recv(buffer, size, cb);
  }

  void send(const uint8_t *buffer, int size, const epoll_send::sended_fn &cb) noexcept {
    RUNTIME_ASSERT(_strand);
    _sender.send(buffer, size, cb);
  }

  completion_strand &strand() noexcept {
    // This strand is only for users of this object but must not be used for operation as
    // this have shorter lifetime than the registrar and can cause issues during shutdown
    RUNTIME_ASSERT(_strand_for_user);
    return *_strand_for_user;
  }

  epoll_service &service() noexcept {
    RUNTIME_ASSERT(_strand);
    return _strand->service();
  }

  io_handle fd() const noexcept { return _descriptor->fd; }

  const ip_endpoint &endpoint() const noexcept { return _ep; }

 private:
  completion_strand *_strand_for_user = nullptr;
  completion_strand_impl *_strand = nullptr;
  reactor_io_descriptor_ptr _descriptor;
  epoll_recv _recver;
  epoll_send _sender;
  ip_endpoint _ep;
};

}  // namespace baba::os