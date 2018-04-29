#工程名称
project(db)
#cmake版本要求检查
cmake_minimum_required(VERSION 3.2.0)
# 设置根目录
SET(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}")
#是否不启用系统Poco，由于我们直接依赖源码包，因此设置为1
option(NOT_UNBUNDLED "not to use system poco library" 1)

#开启c++11,不直接使用add_definitions(-std=c++11)的原因是：
#编译c代码时会有警告:
#cc1: warning: command line option ‘-std=c++11’ is valid for C++/ObjC++ but not for C
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++11")   
endif(CMAKE_COMPILER_IS_GNUCXX)

#打印当前根目录路径（绝对路径）
message("project root path is : " ${PROJECT_ROOT_PATH} )

#执行集成poco的相关cmake命令
include(cmake/find_poco.cmake)

#添加子目录
add_subdirectory(contrib)
add_subdirectory(util)
add_subdirectory(db)
