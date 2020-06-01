/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/impl/bsd_socket_impl.h"
#include "os/impl/epoll_impl.h"
#include "os/impl/unix_file_impl.h"
#include "os/reactor/epoll/epoll_service.h"
#include "os/reactor/epoll/events/epoll_connect.h"
#include "os/socket/socket_option.h"

#include <mutex>

namespace baba::os {

class async_connector final {
 public:
  NOT_COPYABLE(async_connector)

  friend class async_tcp;

  async_connector() = default;

  async_connector(async_connector &&tmp) noexcept
      : _strand_for_user(tmp._strand_for_user),
        _strand(tmp._strand),
        _descriptor(std::move(tmp._descriptor)),
        _connector(std::move(tmp._connector)),
        _connector_ep(std::move(tmp._connector_ep)) {
    tmp._strand_for_user = nullptr;
    tmp._strand = nullptr;
  }

  async_connector &operator=(async_connector &&tmp) noexcept {
    _strand_for_user = tmp._strand_for_user;
    _strand = tmp._strand;
    _descriptor = std::move(tmp._descriptor);
    _connector = std::move(tmp._connector);
    _connector_ep = std::move(tmp._connector_ep);
    tmp._strand_for_user = nullptr;
    tmp._strand = nullptr;
    return *this;
  }

  async_connector(completion_strand &strand, address_family af) noexcept
      : _strand_for_user(&strand),
        _strand(strand.impl()),
        _descriptor(new reactor_io_descriptor()),
        _connector(
            _descriptor, strand.impl(), _strand->service().registrar(),
            _strand->service().deletion_queue(),
            [epoll_fd = _strand->service().reactor().fd()](reactor_event *evt) {
              evt->descriptor->set_write_mask();
              return epoll_impl::arm(epoll_fd, evt->descriptor->fd, evt->descriptor->event_mask,
                                     evt->descriptor.get());
            },
            [](io_handle connector_fd, const ip_endpoint &peer_ep) {
              return socket_impl::connect(connector_fd, peer_ep);
            }) {
    RUNTIME_ASSERT(_strand);
    const auto [e, fd] = socket_impl::create_tcp(af);
    if (e == ec::OK) {
      _descriptor->fd = fd;
    }
  }

  ~async_connector() noexcept {
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
        _connector.dispose();
      }
    }
  }

  void connect(const ip_endpoint &ep, const epoll_connect::connected_fn &cb) noexcept {
    RUNTIME_ASSERT(_strand);
    _connector_ep = ep;
    _connector.connect(_connector_ep, cb);
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

  const ip_endpoint &endpoint() const noexcept { return _connector_ep; }

 private:
  completion_strand *_strand_for_user = nullptr;
  completion_strand_impl *_strand = nullptr;
  reactor_io_descriptor_ptr _descriptor;
  epoll_connect _connector;
  ip_endpoint _connector_ep;
};

}  // namespace baba::os