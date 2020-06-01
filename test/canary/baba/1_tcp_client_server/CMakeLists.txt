# server
add_executable(canary_tcp_server
  server.cpp
)
target_link_libraries(canary_tcp_server PRIVATE baba_io)
target_include_directories(canary_tcp_server PRIVATE ../../utils)

# client
add_executable(canary_tcp_client
  client.cpp
)
target_link_libraries(canary_tcp_client PRIVATE baba_io)
target_include_directories(canary_tcp_client PRIVATE ../../utils)

