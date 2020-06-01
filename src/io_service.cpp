/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "io_service_pimpl.h"

namespace baba {

io_service::io_service() noexcept : _pimpl(new pimpl(0)) {}

io_service::~io_service() noexcept = default;

io_service::io_service(int reactor_threads) noexcept : _pimpl(new pimpl(reactor_threads)) {}

void io_service::run() noexcept { _pimpl->impl.run(); }

void io_service::stop() noexcept { _pimpl->impl.stop(); }

}  // namespace baba