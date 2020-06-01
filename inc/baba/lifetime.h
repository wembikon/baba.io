/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include <memory>

namespace baba {

using lifetime = std::shared_ptr<void>;
using weak_lifetime = std::weak_ptr<void>;

inline lifetime make_default_lifetime() { return std::make_shared<int>(0); }

}  // namespace baba