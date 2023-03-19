/**
 * lua_chat_actions.i
 *
 * This is the SWIG interface file. This file tells SWIG which files
 * to wrap (i.e. make available in Lua) and also contains type mappings
 * plus any additional code that is required to allow the SWIG wrapper
 * to compile.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

%module Actions

%include <std_string.i>

// Definitions required by the SWIG wrapper to compile
%{
#include "lua_mesh_log_manager.hpp"
#include "lua_mesh_action_log.hpp"
#include "lua_mesh_action_scan.hpp"
#include "lua_mesh_action_connector.hpp"
#include "lua_mesh_action_timer.hpp"
%}

// Files to be wrapped by SWIG
%include "lua_mesh_action_log.hpp"

%define CTOR_ERROR
{
  try {
    $function
  }
  catch (std::exception const& e) {
    log_fatal(e.what());
  }
}
%enddef

// Include the actions and define exception handlers
%exception Scan::Scan CTOR_ERROR;
%include "lua_mesh_action_scan.hpp"

%exception Connector::Connector CTOR_ERROR;
%include "lua_mesh_action_connector.hpp"

%exception Timer::Timer CTOR_ERROR;
%include "lua_mesh_action_timer.hpp"
