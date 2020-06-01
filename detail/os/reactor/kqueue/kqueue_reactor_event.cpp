/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/kqueue/kqueue_reactor_event.h"
#include "os/common/event_registrar.h"
#include "os/impl/unix_file_impl.h"

namespace baba {

reactor_io_descriptor::~reactor_io_descriptor() noexcept {
  if (fd != INVALID_HANDLE_VALUE) {
    unix_file_impl::close(fd);
    fd = INVALID_HANDLE_VALUE;
  }
}

}  // namespace baba