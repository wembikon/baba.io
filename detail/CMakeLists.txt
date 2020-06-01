if(PLATFORM_LINUX)
  set(OS_SRCS
    os/impl/unix_file_impl.cpp
    os/impl/epoll_impl.cpp
    os/impl/bsd_socket_impl.cpp
    os/impl/linux_timer_impl.cpp
    os/reactor/epoll/epoll_reactor_event.cpp
    os/reactor/epoll/epoll_reactor.cpp
    os/reactor/epoll/epoll_interrupter.cpp      
    os/reactor/epoll/epoll_service.cpp
    os/reactor/epoll/epoll_completion_strand.cpp
    os/reactor/epoll/events/epoll_recv.cpp
    os/reactor/epoll/events/epoll_send.cpp
    os/reactor/epoll/events/epoll_accept.cpp
    os/reactor/epoll/events/epoll_connect.cpp
    #os/reactor/epoll/events/epoll_recv_from.cpp
    #os/reactor/epoll/events/epoll_send_to.cpp
    os/reactor/epoll/events/epoll_timer.cpp
  )
elseif(PLATFORM_APPLE)
  set(OS_SRCS
    os/impl/unix_file_impl.cpp
    os/impl/kqueue_impl.cpp
    os/impl/bsd_socket_impl.cpp
    os/impl/kqueue_timer_impl.cpp
    os/reactor/kqueue/kqueue_reactor_event.cpp
    os/reactor/kqueue/kqueue_reactor.cpp
    os/reactor/kqueue/kqueue_interrupter.cpp      
    os/reactor/kqueue/kqueue_service.cpp
    os/reactor/kqueue/events/kqueue_recv.cpp
    os/reactor/kqueue/events/kqueue_send.cpp
    os/reactor/kqueue/events/kqueue_accept.cpp
    os/reactor/kqueue/events/kqueue_connect.cpp
    os/reactor/kqueue/events/kqueue_recv_from.cpp
    os/reactor/kqueue/events/kqueue_send_to.cpp
    os/reactor/kqueue/events/kqueue_timer.cpp
  )
elseif(PLATFORM_WINDOWS)
  set(OS_SRCS
    os/impl/windows_file_impl.cpp
    os/impl/iocp_impl.cpp
    os/impl/wsa_socket_impl.cpp
    os/impl/windows_timer_impl.cpp
    os/reactor/iocp/iocp_reactor.cpp
    os/reactor/iocp/iocp_service.cpp
    os/reactor/iocp/wsa_activator.cpp
    os/reactor/iocp/events/iocp_timer.cpp
  )
endif()
add_library(detail STATIC
  ${OS_SRCS}
  os/common/event_registrar.cpp
  os/socket/address_family.cpp
  os/socket/ip_address.cpp
  os/socket/ip_endpoint.cpp
  os/socket/socket_option.cpp
)
target_include_directories(detail PRIVATE ${CMAKE_SOURCE_DIR}/inc)
target_include_directories(detail PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(detail PUBLIC Threads::Threads spdlog::spdlog)
if (MSVC)
  target_compile_options(detail PRIVATE /W3)
else()
  target_compile_options(detail PUBLIC -Wall -Wextra -Wpedantic -fno-rtti -g)
endif()

#---------------------------------------------------------------------------------------
# Sanitizers
#---------------------------------------------------------------------------------------

if (MSVC)
  #TODO: need to have this also on Windows
else()
  if(BUILD_WITH_ASAN_ENABLED)
    message(STATUS "baba.io/detail: Building with address sanitizer")
    set(ASAN_COMPILE_FLAG -O1 -g -fsanitize=address -fno-omit-frame-pointer)
    set(ASAN_LINK_FLAG -g -fsanitize=address)
    target_compile_options(detail PUBLIC ${ASAN_COMPILE_FLAG})
    target_link_libraries(detail PUBLIC ${ASAN_LINK_FLAG})
  endif()
  if(BUILD_WITH_TSAN_ENABLED)
    message(STATUS "baba.io/detail: Building with thread sanitizer")
    set(TSAN_COMPILE_FLAG -O1 -g -fsanitize=thread)
    set(TSAN_LINK_FLAG -g -fsanitize=thread)
    target_compile_options(detail PUBLIC ${TSAN_COMPILE_FLAG})
    target_link_libraries(detail PUBLIC ${TSAN_LINK_FLAG})
  endif()
endif()