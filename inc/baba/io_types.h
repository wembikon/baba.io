/**
 * MIT License
 * Copyright (c) 2020 Adrian T. Visarra
**/

#pragma once

#include "baba/platform.h"

#ifdef PLATFORM_LINUX
#include <errno.h>  // errno
// socket
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>  // send, recv, socket
#include <sys/types.h>   // send, recv, socket
#include <unistd.h>      // close, shutdown
// timer
#include <sys/timerfd.h>
#define HANDLE int
#define SOCKET int
#define INVALID_HANDLE_VALUE -1
#define INVALID_SOCKET -1
#endif  // PLATFORM_LINUX

#ifdef PLATFORM_WINDOWS
#include <WS2tcpip.h>
#include <WinBase.h>
#include <WinSock2.h>
#include <Windows.h>
#include <mswsock.h>
#include <iostream>
// Standard socket API
#pragma comment(lib, "Ws2_32.lib")
// Windows socket extension. AcceptEx etc
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif  // PLATFORM_WINDOWS

#ifdef PLATFORM_APPLE
#include <arpa/inet.h>
#include <errno.h>  // errno
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>  // send, recv, socket
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>  // close, shutdown
#define HANDLE int
#define SOCKET int
#define INVALID_HANDLE_VALUE -1
#define INVALID_SOCKET -1
#endif  // PLATFORM_APPLE