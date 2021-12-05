cmake_minimum_required(VERSION 2.8)
project(tinyrpc)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -lpthread -Wall -Wno-deprecated -Werror -Wno-dev -Wno-unused-function")

set(LIB_SRC
  src/log/log.cc  
  src/net/reactor.cc
  src/net/timer.cc
)

add_library(tinyrpc SHARED ${LIB_SRC})

set(LIBS
  tinyrpc
)

add_executable(test_log testcases/test_log.cc)
add_dependencies(test_log tinyrpc)
target_link_libraries(test_log ${LIBS})

add_executable(test_reactor testcases/test_reactor.cc)
add_dependencies(test_reactor tinyrpc)
target_link_libraries(test_reactor ${LIBS})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

