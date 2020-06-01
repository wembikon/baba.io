/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/epoll/epoll_reactor_event.h"
#include "os/common/event_registrar.h"
#include "os/impl/unix_file_impl.h"

namespace baba {

void reactor_io_descriptor::set_read_mask() noexcept { this->event_mask |= EPOLLIN; }

void reactor_io_descriptor::set_write_mask() noexcept { this->event_mask |= EPOLLOUT; }

void reactor_io_descriptor::clear_read_mask() noexcept { this->event_mask &= ~EPOLLIN; }

void reactor_io_descriptor::clear_write_mask() noexcept { this->event_mask &= ~EPOLLOUT; }

}  // namespace baba