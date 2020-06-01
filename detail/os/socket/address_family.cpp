/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "baba/address_family.h"

#include "baba/io_types.h"

namespace baba {

namespace family {

address_family v4() noexcept { return AF_INET; }

address_family v6() noexcept { return AF_INET6; }

address_family unknown() noexcept { return 0; }

}  // namespace family

}  // namespace baba