/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/error_code.h"
#include "baba/io_strand.h"
#include "baba/semantics.h"

#include <functional>
#include <memory>

namespace baba {

using on_timeout = std::function<void(error_code)>;

class timer final {
 public:
  NOT_COPYABLE(timer)

  struct pimpl;

  timer(timer &&tmp) noexcept;
  timer &operator=(timer &&tmp) noexcept;
  timer() noexcept;
  ~timer() noexcept;

  timer(io_strand &strand) noexcept;
  void timeout(uint32_t timeout_ms, const on_timeout &cb) noexcept;
  io_strand &strand() noexcept;

 private:
  io_strand *_strand = nullptr;
  std::unique_ptr<pimpl> _pimpl;
};

}  // namespace baba