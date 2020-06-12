cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION 2.0.0)
project(baba.io VERSION ${PROJECT_VERSION} LANGUAGES CXX)
message(STATUS "Build baba.io: ${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_WITH_ASAN "Build with ASan" ON)
option(BUILD_WITH_TSAN "Build with TSan" OFF)
option(BUILD_CANARY_TESTS "Build canary tests" ON)
option(BUILD_UNIT_TESTS "Build unit tests" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if(BUILD_WITH_ASAN)
    set(BUILD_WITH_ASAN_ENABLED ON)
  endif()
  if(BUILD_WITH_TSAN)
    set(BUILD_WITH_TSAN_ENABLED ON)
  endif()
  if(BUILD_CANARY_TESTS)
    set(BUILD_CANARY_TESTS_ENABLED ON)
  endif()
  if(BUILD_UNIT_TESTS)
    set(BUILD_UNIT_TESTS_ENABLED ON)
  endif()
endif()

message(STATUS "Platform is ${CMAKE_SYSTEM}")
if("${CMAKE_SYSTEM}" MATCHES "Linux")
  set(PLATFORM_LINUX ON)
elseif("${CMAKE_SYSTEM}" MATCHES "Darwin")
  set(PLATFORM_APPLE ON)
elseif("${CMAKE_SYSTEM}" MATCHES "Windows")
  set(PLATFORM_WINDOWS ON)
endif()

# Definitions must be defined before adding libraries so that they gets applied to all
# or else you will get weird errors like pointers passed to a lambda that is defined in
# a header, won't have its field populated
if(BUILD_UNIT_TESTS_ENABLED)
  if (MSVC)
    add_definitions(/DBUILD_UNIT_TESTS)
  else()
    add_definitions(-DBUILD_UNIT_TESTS)
  endif()
endif()

find_package(Threads REQUIRED)
add_subdirectory(thirdparty/spdlog EXCLUDE_FROM_ALL)
add_subdirectory(detail)

#---------------------------------------------------------------------------------------
# Product
#---------------------------------------------------------------------------------------
add_library(baba_io STATIC
  src/ip_address.cpp
  src/ip_endpoint.cpp
  src/socket_option.cpp
  src/io_service.cpp
  src/io_strand.cpp
  src/timer.cpp
  src/tcp_connector.cpp
  src/tcp_socket.cpp
  src/tcp_acceptor.cpp
  src/cancellable_task.cpp
)
# Expose usage requirements accordingly for `build` and `install` clients
target_include_directories(baba_io PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/pimpl>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(baba_io PRIVATE ${CMAKE_SOURCE_DIR}/detail)
target_link_libraries(baba_io PRIVATE baba_io_detail)
target_link_libraries(baba_io PUBLIC Threads::Threads spdlog::spdlog)
if (MSVC)
  target_compile_options(baba_io PRIVATE /W3)
else()
  target_compile_options(baba_io PUBLIC -Wall -Wextra -Wpedantic -fno-rtti -g)
endif()

#---------------------------------------------------------------------------------------
# Sanitizers
#---------------------------------------------------------------------------------------
if (MSVC)
  #TODO: need to have this also on Windows
else()
  if(BUILD_WITH_ASAN_ENABLED)
    message(STATUS "baba.io: Building with address sanitizer")
    set(ASAN_COMPILE_FLAG -O1 -g -fsanitize=address -fno-omit-frame-pointer)
    set(ASAN_LINK_FLAG -g -fsanitize=address)
    target_compile_options(baba_io PUBLIC ${ASAN_COMPILE_FLAG})
    target_link_libraries(baba_io PUBLIC ${ASAN_LINK_FLAG})
  endif()
  if(BUILD_WITH_TSAN_ENABLED)
    message(STATUS "baba.io: Building with thread sanitizer")
    set(TSAN_COMPILE_FLAG -O1 -g -fsanitize=thread)
    set(TSAN_LINK_FLAG -g -fsanitize=thread)
    target_compile_options(baba_io PUBLIC ${TSAN_COMPILE_FLAG})
    target_link_libraries(baba_io PUBLIC ${TSAN_LINK_FLAG})
  endif()
endif()

#---------------------------------------------------------------------------------------
# Tests
#---------------------------------------------------------------------------------------
if(BUILD_CANARY_TESTS_ENABLED)
  message(STATUS "baba.io: Canary test builds enabled")
  add_subdirectory(test/canary/baba/1_tcp_client_server)
endif()

#---------------------------------------------------------------------------------------
# Detail Unit Tests
#---------------------------------------------------------------------------------------
if(BUILD_UNIT_TESTS_ENABLED)
  message(STATUS "baba.io: Unit test builds enabled")
  enable_testing()
  if (MSVC)
    # Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  endif()
  add_subdirectory(thirdparty/googletest EXCLUDE_FROM_ALL)

  set(UT_SRCS)
  if(PLATFORM_LINUX)
    add_subdirectory(test/unit/detail/epoll)
  elseif(PLATFORM_APPLE)
    add_subdirectory(test/unit/detail/kqueue)
  elseif(PLATFORM_WINDOWS)
    add_subdirectory(test/unit/detail/iocp)
  endif()

  add_executable(baba_ut
    ${UT_SRCS}
    test/unit/main.cpp
  )
  target_link_libraries(baba_ut PRIVATE baba_io baba_io_detail gtest gtest_main gmock gmock_main)
  add_test(NAME baba_ut COMMAND baba_ut)
endif()

#---------------------------------------------------------------------------------------
# Install
#---------------------------------------------------------------------------------------
set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME})
message(STATUS "Installation directory ${INSTALL_DIR}")

set(EXPORT_DIR "${INSTALL_DIR}/cmake")
set(EXPORTED_TARGETS_NAME "baba.ioTargets")
set(TARGETS_FILE "baba.ioTargets.cmake")
set(CONFIG_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/baba.ioConfigVersion.cmake")
set(CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/baba.ioConfig.cmake")

set(PKGCONFIG_INSTALL_DIR "${INSTALL_DIR}/pkgconfig")
set(PKGCONFIG_FILE "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc")

# Include files
install(DIRECTORY inc/baba DESTINATION "${INSTALL_DIR}/include")
install(
  TARGETS baba_io baba_io_detail spdlog
  EXPORT ${EXPORTED_TARGETS_NAME}
  DESTINATION "${INSTALL_DIR}/lib"
)

# API visibility
include(GenerateExportHeader)
generate_export_header(baba_io)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/baba_io_export.h"
  DESTINATION "${INSTALL_DIR}/include"
)

# cmake config
install(
  EXPORT ${EXPORTED_TARGETS_NAME} 
  FILE "${TARGETS_FILE}"
  NAMESPACE baba::io:: 
  DESTINATION "${EXPORT_DIR}"
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CONFIG_VERSION_FILE}"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

export(
  EXPORT ${EXPORTED_TARGETS_NAME}
  FILE "${CONFIG_FILE}"
  NAMESPACE baba::io::
)

install(
  FILES "${CONFIG_VERSION_FILE}" "${CONFIG_FILE}"
  DESTINATION "${EXPORT_DIR}"
)

# pkgconfig
get_target_property(PKG_CONFIG_DEFINES baba_io INTERFACE_COMPILE_DEFINITIONS)
string(REPLACE ";" " -D" PKG_CONFIG_DEFINES "${PKG_CONFIG_DEFINES}")
string(CONCAT PKG_CONFIG_DEFINES "-D" "${PKG_CONFIG_DEFINES}")
configure_file("cmake/${PROJECT_NAME}.pc.in" "${PKGCONFIG_FILE}" @ONLY)
install(FILES "${PKGCONFIG_FILE}" DESTINATION "${PKGCONFIG_INSTALL_DIR}")