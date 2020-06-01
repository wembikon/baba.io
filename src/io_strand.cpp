/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "io_strand_pimpl.h"

namespace baba {

io_strand::~io_strand() noexcept = default;

io_strand::io_strand(io_service &service) noexcept
    : _service(service), _pimpl(new pimpl(service)) {}

void io_strand::post(task_fn &&task) noexcept { _pimpl->impl.post(std::move(task)); }

io_service &io_strand::service() noexcept { return _service; }

}  // namespace baba