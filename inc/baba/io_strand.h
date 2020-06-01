/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/io_service.h"
#include "baba/semantics.h"

#include <functional>
#include <memory>

namespace baba {

class io_strand final {
 public:
  NOT_COPYABLE(io_strand)
  NOT_MOVEABLE(io_strand)

  struct pimpl;

  using task_fn = std::function<void()>;

  ~io_strand() noexcept;

  io_strand(io_service &service) noexcept;
  void post(task_fn &&task) noexcept;
  io_service &service() noexcept;

 private:
  friend class timer;
  friend class tcp_connector;
  friend class tcp_socket;
  friend class tcp_acceptor;
  io_service &_service;
  std::unique_ptr<pimpl> _pimpl;
};

}  // namespace baba