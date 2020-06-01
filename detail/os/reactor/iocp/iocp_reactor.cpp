/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/iocp/iocp_reactor.h"

#include "baba/logger.h"
#include "baba/semantics.h"
#include "os/impl/iocp_impl.h"
#include "os/impl/windows_file_impl.h"

namespace baba::os {

iocp_reactor::iocp_reactor() noexcept {
  const auto [e, fd] = iocp_impl::create();
  if (e != ec::OK) {
    LOGFTL("Cannot instantiate iocp");
  }
  _fd = fd;
}

error_code iocp_reactor::wait() noexcept {
  if (const auto [e, ready] = iocp_impl::wait(_fd, _event_list, MAX_IOCP_ENTRIES); e == ec::OK) {
    for (ULONG i = 0; i < ready; ++i) {
      const auto evt = static_cast<reactor_event *>(_event_list[i].lpOverlapped);
      evt->react();
    }
    return ec::OK;
  } else {
    return e;
  }
}

void iocp_reactor::close() noexcept { windows_file_impl::close(_fd); }

io_handle iocp_reactor::fd() const noexcept { return _fd; }

}  // namespace baba::os