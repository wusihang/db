project(db)
cmake_minimum_required(VERSION 3.2.0)
# set project root path
SET(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}")
option(NOT_UNBUNDLED "not to use system poco library" 1)

# enable c++11  , only for c++ compile . 
# if not judge , then console will produce the following warning:
# cc1: warning: command line option '-std=c++11' is valid for C++/ObjC++ but not for C
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")   
endif(CMAKE_COMPILER_IS_GNUCXX)

message("project root path is : " ${PROJECT_ROOT_PATH} )

include(cmake/find_poco.cmake)

add_subdirectory(contrib)
add_subdirectory(util)
add_subdirectory(db)
