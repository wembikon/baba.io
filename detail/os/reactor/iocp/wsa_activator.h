/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/io_types.h"

namespace baba::os {

class wsa_activator {
 public:
  wsa_activator();
  ~wsa_activator();

 private:
  WSAData _data;
};

}  // namespace baba::os