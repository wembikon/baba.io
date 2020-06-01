/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "os/impl/bsd_socket_impl.h"
#include "os/impl/epoll_impl.h"
#include "os/impl/unix_file_impl.h"
#include "os/reactor/epoll/epoll_service.h"
#include "os/reactor/epoll/events/epoll_accept.h"

#include "os/reactor/epoll/io/epoll_async_tcp.h"

#include <mutex>

namespace baba::os {

class async_acceptor final {
 public:
  NOT_COPYABLE(async_acceptor)

  async_acceptor() = default;

  async_acceptor(async_acceptor &&tmp) noexcept
      : _strand_for_user(tmp._strand_for_user),
        _strand(tmp._strand),
        _descriptor(std::move(tmp._descriptor)),
        _acceptor(std::move(tmp._acceptor)),
        _acceptor_ep(std::move(tmp._acceptor_ep)),
        _peer_fd(tmp._peer_fd),
        _peer_ep(std::move(tmp._peer_ep)) {
    tmp._strand_for_user = nullptr;
    tmp._strand = nullptr;
    tmp._peer_fd = INVALID_HANDLE_VALUE;
  }

  async_acceptor &operator=(async_acceptor &&tmp) noexcept {
    _strand_for_user = tmp._strand_for_user;
    _strand = tmp._strand;
    _descriptor = std::move(tmp._descriptor);
    _acceptor = std::move(tmp._acceptor);
    _acceptor_ep = std::move(tmp._acceptor_ep);
    _peer_fd = tmp._peer_fd;
    _peer_ep = std::move(tmp._peer_ep);
    tmp._strand_for_user = nullptr;
    tmp._strand = nullptr;
    tmp._peer_fd = INVALID_HANDLE_VALUE;
    return *this;
  }

  async_acceptor(completion_strand &strand, int af) noexcept
      : _strand_for_user(&strand),
        _strand(strand.impl()),
        _descriptor(new reactor_io_descriptor()),
        _acceptor(
            _descriptor, _strand, _strand->service().registrar(),
            _strand->service().deletion_queue(),
            [epoll_fd = _strand->service().reactor().fd()](reactor_event *evt) {
              evt->descriptor->set_read_mask();
              return epoll_impl::arm(epoll_fd, evt->descriptor->fd, evt->descriptor->event_mask,
                                     evt->descriptor.get());
            },
            [](io_handle acceptor_fd, const ip_endpoint &acceptor_ep, io_handle &peer_fd,
               ip_endpoint &peer_ep) {
              const auto [e, pfd, pep] = socket_impl::accept(acceptor_fd, acceptor_ep);
              peer_fd = pfd;
              peer_ep = pep;
              return e;
            }) {
    RUNTIME_ASSERT(_strand);
    const auto [e, fd] = socket_impl::create_tcp(af);
    if (e == ec::OK) {
      _descriptor->fd = fd;
    }
  }

  ~async_acceptor() noexcept {
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
        _acceptor.dispose();
      }
    }
  }

  error_code bind(const ip_endpoint &ep) noexcept {
    _acceptor_ep = ep;
    return socket_impl::bind(_descriptor->fd, _acceptor_ep);
  }

  error_code listen(int backlog) noexcept { return socket_impl::listen(_descriptor->fd, backlog); }

  void accept(async_tcp &peer, const epoll_accept::accepted_fn &cb) noexcept {
    RUNTIME_ASSERT(_strand);
    _acceptor.accept(_acceptor_ep, _peer_fd, _peer_ep, [this, &peer, cb](error_code e) {
      if (e == ec::OK) {
        peer = async_tcp(*_strand_for_user, _peer_fd, _peer_ep);
      }
      cb(e);
    });
  }

  void accept(completion_strand &peer_strand, async_tcp &peer,
              const epoll_accept::accepted_fn &cb) noexcept {
    _acceptor.accept(_acceptor_ep, _peer_fd, _peer_ep,
                     [this, &peer_strand, &peer, cb](error_code e) {
                       if (e == ec::OK) {
                         peer = async_tcp(peer_strand, _peer_fd, _peer_ep);
                       }
                       cb(e);
                     });
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

  const ip_endpoint &endpoint() const noexcept { return _acceptor_ep; }

 private:
  completion_strand *_strand_for_user = nullptr;
  completion_strand_impl *_strand = nullptr;
  reactor_io_descriptor_ptr _descriptor;
  epoll_accept _acceptor;
  ip_endpoint _acceptor_ep;
  io_handle _peer_fd = INVALID_HANDLE_VALUE;
  ip_endpoint _peer_ep;
};

}  // namespace baba::os