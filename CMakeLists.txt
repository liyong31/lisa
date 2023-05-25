cmake_minimum_required(VERSION 2.6)
project(lisa-syntcomp)
set(PROJECT lisa-syntcomp)
set(VERSION 0.0.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pthread")

set(THIRD_PARTIES "${PROJECT_SOURCE_DIR}/third_parties")

#include_directories("${THIRD_PARTIES}/spdlog/build")
#include_directories("${THIRD_PARTIES}/aiger-1.9.4/")
# for cudd, we need the mtr header, that is why we reference cudd using this hack
# (instead of using expected `configure --prefix && make && make install`, which does not instll the mtr header)
#include_directories("${THIRD_PARTIES}/cudd-3.0.0/cudd/" "${THIRD_PARTIES}/cudd-3.0.0/cplusplus/" "${THIRD_PARTIES}/cudd-3.0.0/mtr/")
include_directories("${THIRD_PARTIES}/args/")
include_directories("/usr/include/" "/usr/include/spot")
include_directories("${THIRD_PARTIES}/pstreams-1.0.3/")


#list(APPEND LIBS "${THIRD_PARTIES}/cudd-3.0.0/cudd/.libs/libcudd.a")
#list(APPEND LIBS "${THIRD_PARTIES}/aiger-1.9.4/aiger.o")
list(APPEND LIBS "spot" "bddx")

if(EXISTS "${THIRD_PARTIES}/spot-install-prefix/")
    message("\n ************* I am going to use locally installed spot **************** \n")
    include_directories("${THIRD_PARTIES}/spot-install-prefix/include/")
    link_directories("${THIRD_PARTIES}/spot-install-prefix/lib/")
endif()

add_subdirectory(src)

#enable_testing()
#add_subdirectory(func_tests)
