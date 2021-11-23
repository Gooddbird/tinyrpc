cmake_minimum_required(VERSION 2.8)
project(tinyrpc)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -lpthread -Wall -Wno-deprecated -Werror -Wno-dev -Wno-unused-function")

set(LIB_SRC
    src/net/reactor.cc
)

add_library(tinyprc SHARED ${LIB_SRC})

set(LIBS

    tinyrpc
)

# add_executable(test_log tests/test_log.cc)
# add_dependencies(test_log charon)
# target_link_libraries(test_log ${LIBS})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

