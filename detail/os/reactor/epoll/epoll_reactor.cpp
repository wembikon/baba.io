/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/epoll_reactor.h"

#include "baba/logger.h"
#include "baba/semantics.h"
#include "os/impl/epoll_impl.h"
#include "os/impl/unix_file_impl.h"

#include <signal.h>

namespace baba::os {

epoll_reactor::epoll_reactor(const enqueue_reactor_task_fn &enqueue_reactor_task) noexcept
    : _enqueue_reactor_task(enqueue_reactor_task) {
  // Prevent ::send from aborting the program when it receives a SIGPIPE error
  signal(SIGPIPE, SIG_IGN);

  const auto [e, fd] = epoll_impl::create();
  if (e != ec::OK) {
    LOGFTL("Cannot instantiate epoll");
  }
  _fd = fd;
}

error_code epoll_reactor::wait() noexcept {
  if (const auto [e, ready] = epoll_impl::wait(_fd, _event_list, MAX_EPOLL_EVENTS); e == ec::OK) {
    for (int i = 0; _run.load() && i < ready; ++i) {
      const auto io_desc = static_cast<reactor_io_descriptor *>(_event_list[i].data.ptr);
      // We must determine if we should rearm before we call react on the descriptor in order
      // to only call react once - if we have error during rearming then we call react only
      // for this error, else then we can proceed.
      if (_event_list[i].events & EPOLLIN) {
        if (io_desc->fd == _interrupter_fd) {
          // Consume the interrupter buffer
          uint8_t byte[1024];
          while (true) {
            if (const auto [e, b] = unix_file_impl::read(_interrupter_fd, byte, 1024 - 1);
                e != ec::OK) {
              if (e == EAGAIN) {  // no more bytes
                break;
              } else {  // other errors
                LOGFTL("Consuming self pipe failed. ec={}", e);
              }
            }
          }
          continue;
        }
        io_desc->clear_read_mask();
      }
      if (_event_list[i].events & EPOLLOUT) {
        io_desc->clear_write_mask();
      }
      // EPOLLONESHOT requires us to re-arm in case we still have remaining events
      if (io_desc->event_mask != 0) {
        const auto ec = epoll_impl::arm(_fd, io_desc->fd, io_desc->event_mask, io_desc);
        // If we encounter an error here, we need to deliver it to all the existing callbacks
        if (ec != ec::OK) {
          if (io_desc->read_event) {
            // We set it before we enqueue to the reactor_task queue because by then we cannot
            // assure the sequence they are going to be processed anymore and there are cases
            // that the final event is completed before even we can set this flag causing a
            // heap-use-after-free. Ditto on other lines that set this flag.
            io_desc->read_event->has_active_event = true;
            _enqueue_reactor_task([io_desc]() { io_desc->read_event->react(EBADF); });
          }
          if (io_desc->write_event) {
            io_desc->write_event->has_active_event = true;
            _enqueue_reactor_task([io_desc]() { io_desc->write_event->react(EBADF); });
          }
          // Must not call react again!
          continue;
        }
      }

      if (_event_list[i].events & EPOLLIN) {
        io_desc->read_event->has_active_event = true;
        if (_event_list[i].events & EPOLLHUP) {
          _enqueue_reactor_task([io_desc]() { io_desc->read_event->react(EPOLLHUP); });
        } else {
          _enqueue_reactor_task([io_desc]() { io_desc->read_event->react(ec::OK); });
        }
      }
      if (_event_list[i].events & EPOLLOUT) {
        io_desc->write_event->has_active_event = true;
        if (_event_list[i].events & EPOLLERR) {
          _enqueue_reactor_task([io_desc]() { io_desc->write_event->react(EPOLLERR); });
        } else {
          _enqueue_reactor_task([io_desc]() { io_desc->write_event->react(ec::OK); });
        }
      }
    }
    return ec::OK;
  } else {
    return e;
  }
}

void epoll_reactor::stop() noexcept { _run.store(false); }

void epoll_reactor::close() noexcept { unix_file_impl::close(_fd); }

io_handle epoll_reactor::fd() const noexcept { return _fd; }

void epoll_reactor::set_interrupter_fd(io_handle fd) noexcept { _interrupter_fd = fd; }

}  // namespace baba::os