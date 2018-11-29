cmake_minimum_required(VERSION 3.0.0)
project(sunshine VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(sunshine main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
