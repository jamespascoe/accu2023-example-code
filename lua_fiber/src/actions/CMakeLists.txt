#[[

CMakeLists.txt - 'lua_fiber' actions subdirectory build generation file.

Copyright © Blu Wireless. All Rights Reserved.
Licensed under the MIT license. See LICENSE file in the project.
Feedback: james.pascoe@bluwireless.com

]]

set(LUA_FIBER_SWIG_SRCS
    lua_fiber_actions.i
    lua_fiber_action_log.cpp
    lua_fiber_action_connector.cpp
    lua_fiber_action_timer.cpp)

set_source_files_properties(${LUA_FIBER_SWIG_SRCS} PROPERTIES CPLUSPLUS ON)
swig_add_library(actions
                 TYPE USE_BUILD_SHARED_LIBS
                 LANGUAGE lua
                 SOURCES ${LUA_FIBER_SWIG_SRCS})

target_include_directories(actions PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                           ${LUA_FIBER_SOURCE_DIR}/src
                                           ${LUA_FIBER_SOURCE_DIR}/third_party
                                           ${LUA_FIBER_SOURCE_DIR}/third_party/asio
                                           ${LUA_INCLUDE_DIR})

target_compile_definitions(actions PRIVATE ASIO_STANDALONE)

target_link_libraries(actions PRIVATE std::filesystem)
