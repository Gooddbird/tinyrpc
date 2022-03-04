cmake_minimum_required(VERSION 2.8)
project(tinyrpc)
set(CMAKE_VERBOSE_MAKEFILE ON)
ENABLE_LANGUAGE(ASM)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-dev -Wno-unused-function")

set(LIB_SRC
  src/log/log.cc  
  src/coroutine/coctx_swap.S
  src/coroutine/coroutine.cc
  src/coroutine/coroutine_hook.cc
  src/net/fd_event.cc
  src/net/reactor.cc
  src/net/timer.cc
  src/net/net_address.cc
  src/net/tcp/tcp_server.cc
  src/net/tcp/tcp_buffer.cc
  src/net/tcp/tcp_connection.cc
  src/net/http/http_codec.cc
  src/net/tinypb/tinypb_codec.cc

)

add_library(tinyrpc SHARED ${LIB_SRC})

set(LIBS
  tinyrpc
  dl
  pthread
)

add_executable(test_log testcases/test_log.cc)
add_dependencies(test_log tinyrpc)
target_link_libraries(test_log ${LIBS})

#add_executable(test_reactor testcases/test_reactor.cc)
#add_dependencies(test_reactor tinyrpc)
#target_link_libraries(test_reactor ${LIBS})


add_executable(test_iothread testcases/test_iothread.cc)
add_dependencies(test_iothread tinyrpc)
target_link_libraries(test_iothread ${LIBS})

add_executable(test_tcpserver testcases/test_tcpserver.cc)
add_dependencies(test_tcpserver tinyrpc)
target_link_libraries(test_tcpserver ${LIBS})

add_executable(test_connect testcases/test_connect.cc)
add_dependencies(test_connect tinyrpc)
target_link_libraries(test_connect ${LIBS})

#add_executable(test_coroutine testcases/test_coroutine.cc)
#add_dependencies(test_coroutine tinyrpc)
#target_link_libraries(test_coroutine ${LIBS})


add_executable(test_coroutine_hook testcases/test_coroutine_hook.cc)
add_dependencies(test_coroutine_hook tinyrpc)
target_link_libraries(test_coroutine_hook ${LIBS})

add_executable(test_tcpserver_cor testcases/test_tcpserver_cor.cc)
add_dependencies(test_tcpserver_cor tinyrpc)
target_link_libraries(test_tcpserver_cor ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

