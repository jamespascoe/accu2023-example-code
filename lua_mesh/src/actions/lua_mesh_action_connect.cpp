/**
 * lua_mesh_action_scan.cpp
 *
 * This action allows Lua behaviours to 'connect' to other nodes that
 * are connected to the Mesh.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#include "lua_mesh_action_connect.hpp"

#include "lua_mesh_log_manager.hpp"

Connect::Connect() {
  log_trace("Connect action starting");
}

Connect::~Connect() {
  log_trace("Connect action exiting");
}

void Connect::DoConnect() {
  return;
}
