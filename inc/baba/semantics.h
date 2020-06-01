/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#define NOT_COPYABLE(TypeName)        \
  TypeName(TypeName const&) = delete; \
  TypeName& operator=(TypeName const&) = delete;

#define NOT_MOVEABLE(TypeName)   \
  TypeName(TypeName&&) = delete; \
  TypeName& operator=(TypeName&&) = delete;

#define DEFAULT_MOVEABLE(TypeName) \
  TypeName(TypeName&&) = default;  \
  TypeName& operator=(TypeName&&) = default;

#define DEFAULT_MOVEABLE_ONLY(TypeName) \
  NOT_COPYABLE(TypeName)                \
  DEFAULT_MOVEABLE(TypeName)

#if CMAKE_BUILD_TYPE == Debug
#include "baba/logger.h"
#define RUNTIME_ASSERT(cond) assert(cond)
#define RUNTIME_ASSERTM(cond, msg, ...) \
  do {                                  \
    if (!(cond)) {                      \
      LOGFTL(msg, ##__VA_ARGS__);       \
      exit(0);                          \
    }                                   \
  } while (0)
#else
#define RUNTIME_ASSERT ((void)0)
#define RUNTIME_ASSERTM ((void)0)
#endif