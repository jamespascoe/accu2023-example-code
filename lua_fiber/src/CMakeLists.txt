#[[

CMakeLists.txt - 'lua_fiber' src subdirectory build generation file.

Copyright © Blu Wireless. All Rights Reserved.
Licensed under the MIT license. See LICENSE file in the project.
Feedback: james.pascoe@bluwireless.com

]]

add_subdirectory(actions)

add_executable(lua_fiber
               lua_fiber_main.cpp
               lua_fiber_log_manager.cpp
               lua_fiber_lua_manager.cpp)

target_link_libraries(lua_fiber
                      PRIVATE
                      actions
                      ${LUA_LIBRARIES}
                      std::filesystem
                      Threads::Threads)

target_include_directories(lua_fiber
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${LUA_INCLUDE_DIR}
                           ${LUA_FIBER_SOURCE_DIR}/third_party)
