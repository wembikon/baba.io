/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/impl/kqueue_timer_impl.h"

namespace baba::os {

uint16_t kqueue_timer_impl::_ident_generator = 1;

io_handle kqueue_timer_impl::create() noexcept {
  return static_cast<io_handle>(_ident_generator++);
}

}  // namespace baba::os