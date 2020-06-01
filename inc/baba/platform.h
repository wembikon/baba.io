/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#ifdef _WIN32
// define something for Windows (32-bit and 64-bit, this part is common)
#define PLATFORM_WINDOWS
#define USE_IOCP

#ifdef _WIN64
// define something for Windows (64-bit only)
#else
// define something for Windows (32-bit only)
#endif
#elif __APPLE__
#define PLATFORM_APPLE
#define USE_KQUEUE

#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS simulator
#elif TARGET_OS_IPHONE
// iOS device
#elif TARGET_OS_MAC
// Other kinds of Mac OS
#else
#error "Unknown Apple platform"
#endif
#elif __linux__
#define PLATFORM_LINUX
#define USE_EPOLL
#elif __unix__  // all unices not caught above
#error "Unix is not yet supported"
#elif defined(_POSIX_VERSION)
#error "Posix is not yet supported"
#else
#error "Unknown compiler"
#endif