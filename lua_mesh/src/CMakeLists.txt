#[[

CMakeLists.txt - 'Luamesh' src subdirectory build generation file.

Copyright © Blu Wireless. All Rights Reserved.
Licensed under the MIT license. See LICENSE file in the project.
Feedback: james.pascoe@bluwireless.com

]]

add_subdirectory(actions)

add_executable(lua_mesh
               lua_mesh_main.cpp
               lua_mesh_log_manager.cpp
               lua_mesh_lua_manager.cpp)

target_link_libraries(lua_mesh
                      PRIVATE
                      actions
                      ${LUA_LIBRARIES}
                      std::filesystem
                      Threads::Threads)

target_include_directories(lua_mesh
                           PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${LUA_INCLUDE_DIR}
                           ${LUA_MESH_SOURCE_DIR}/third_party)
