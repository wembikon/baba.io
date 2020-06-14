/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
 **/

#include "timer_pimpl.h"

namespace baba {

timer::timer(timer &&tmp) noexcept : _strand(tmp._strand), _pimpl(std::move(tmp._pimpl)) {}

timer &timer::operator=(timer &&tmp) noexcept {
  _strand = tmp._strand;
  _pimpl = std::move(tmp._pimpl);
  return *this;
}

timer::timer() noexcept : _pimpl(new pimpl()) {}

timer::~timer() noexcept = default;

timer::timer(io_strand &strand) noexcept : _pimpl(new pimpl(strand)) {}

void timer::timeout(uint32_t timeout_ms, const on_timeout &cb) noexcept {
  _pimpl->impl.timeout(timeout_ms, cb);
}

io_strand &timer::strand() noexcept {
  RUNTIME_ASSERT(_strand);
  return *_strand;
}

}  // namespace baba