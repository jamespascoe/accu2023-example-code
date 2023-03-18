#[[

CMakeLists.txt - 'LuaMesh' top level build generation file.

Copyright © Blu Wireless. All Rights Reserved.
Licensed under the MIT license. See LICENSE file in the project.
Feedback: james.pascoe@bluwireless.com

]]

cmake_minimum_required (VERSION 3.12)
project(LUA_MESH DESCRIPTION
        "Lua Mesh - A Lua Based Tool for Simulating Mobile Mesh Networks")

# Look for required packages
if (POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

if (POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

find_package(SWIG 3.0 REQUIRED)
include(${SWIG_USE_FILE})

find_package(Lua 5.3 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include(third_party/pmm/pmm.cmake)
pmm(CMakeCM ROLLING)

find_package(Filesystem REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)

add_subdirectory(src)
