/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#include "os/reactor/iocp/wsa_activator.h"

#include "baba/logger.h"

#include <cstdlib>

namespace baba::os {

wsa_activator::wsa_activator() : _data{0} {
  if (int result = WSAStartup(MAKEWORD(2, 2), &_data); result != 0) {
    LOGFTL("WSAStartup() failed. result={}", result);
    std::abort();
  }
}

wsa_activator::~wsa_activator() { WSACleanup(); }

}  // namespace baba::os