/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

namespace baba {

using address_family = int;

namespace family {

address_family v4() noexcept;
address_family v6() noexcept;
address_family unknown() noexcept;

}  // namespace family

}  // namespace baba