/**
 * lua_mesh_action_scan.cpp
 *
 * This action allows Lua behaviours to 'Scan' to other nodes that
 * are Scaned to the Mesh.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#include "lua_mesh_action_scan.hpp"

#include "lua_mesh_log_manager.hpp"

Scan::Scan() {
  log_trace("Scan action starting");
}

Scan::~Scan() {
  log_trace("Scan action exiting");
}

void Scan::DoScan() {
  return;
}
