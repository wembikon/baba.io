/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

namespace baba {

using error_code = int;

namespace ec {

enum system_error_code { OK = 0, ERR = 1, EC_PEER_SOCKET_CLOSED = 9000, LAST_EC };

}

}  // namespace baba