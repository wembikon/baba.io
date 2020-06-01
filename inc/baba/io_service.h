/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/semantics.h"

#include <memory>

namespace baba {

class io_service final {
 public:
  NOT_COPYABLE(io_service)
  NOT_MOVEABLE(io_service)

  struct pimpl;

  io_service() noexcept;
  ~io_service() noexcept;

  io_service(int reactor_threads) noexcept;
  void run() noexcept;
  void stop() noexcept;

 private:
  friend class io_strand;
  std::unique_ptr<pimpl> _pimpl;
};

}  // namespace baba