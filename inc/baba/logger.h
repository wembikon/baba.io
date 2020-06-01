/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "spdlog/spdlog.h"

#ifndef ENABLE_LOGTRC
template<typename... Args> void BLACKHOLE([[maybe_unused]] Args && ... args){}
#define LOGTRC(...) BLACKHOLE(__VA_ARGS__)
#else
#define LOGTRC SPDLOG_TRACE
#endif 

#define LOGDBG SPDLOG_DEBUG
#define LOGINF SPDLOG_INFO
#define LOGERR SPDLOG_ERROR
#define LOGFTL SPDLOG_CRITICAL