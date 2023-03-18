/**
 * lua_chat_lua_manager.cpp
 *
 * Implementation of the Lua manager class.
 *
 * This file implements the Lua manager i.e. the piece of software that manages
 * the interactions with the Lua interpreter and the different 'behaviours'.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#include <filesystem>
#include <regex>
#include <stdexcept>
#include <string>

#include <lua.hpp>

#include "lua_mesh_log_manager.hpp"
#include "lua_mesh_lua_manager.hpp"

// Declare the prototype to import the actions library.
extern "C" {
int luaopen_Actions(lua_State* L);
}

LuaManager::LuaManager() {
  lua_state = luaL_newstate();
  if (!lua_state)
    throw std::runtime_error("Unable to initialise Lua state");

  // Open the standard Lua libraries
  luaL_openlibs(lua_state);

  // Open the actions library (i.e. the SWIG bindings for the C++ actions)
  luaopen_Actions(lua_state);
}

// Constructor to load behaviour
LuaManager::LuaManager(std::string const& file_name) : LuaManager() {
  namespace fs = std::filesystem;

  try {
    if (fs::is_regular_file(file_name) &&
        fs::path(file_name).extension() == behaviour_file_extension)
          LoadBehaviour(file_name);
  } catch (fs::filesystem_error& e) {
    log_fatal("Error loading behaviour: {}", e.what());
  }
}

// RAII destructor - ensure that the Lua state is always freed.
LuaManager::~LuaManager() {
  if (!lua_state)
    return;

  // Free all references
  for (auto const& behaviour : behaviour_index)
    if (behaviour.second.entry_point_ref != LUA_NOREF)
      luaL_unref(
          lua_state, LUA_REGISTRYINDEX, behaviour.second.entry_point_ref);

  if (lua_ref_argument_table != LUA_NOREF)
    luaL_unref(lua_state, LUA_REGISTRYINDEX, lua_ref_argument_table);

  lua_close(lua_state);
}

// Load a behaviour. Throw if there are any issues loading or calling the code.
void LuaManager::LoadBehaviour(std::string const& file_name) {
  int lua_ret = LUA_OK;

  // Extract the path from the file_name
  auto path = std::filesystem::path(file_name);

  if (!path.has_parent_path())
    throw std::runtime_error("can not get parent path for " + file_name);

  // Set the package path to include the directory that this behaviour is in.
  // This allows modules that are in the same directory to be imported without
  // having to specify explicit paths or setting environment variables.
  SetLuaPackagePath(path.parent_path());

  // Load and execute the file
  lua_ret = luaL_loadfile(lua_state, file_name.c_str());
  if (lua_ret != LUA_OK)
    ProcessLuaError(lua_ret);

  lua_ret = lua_pcall(lua_state, 0, LUA_MULTRET, 0);
  if (lua_ret != LUA_OK)
    ProcessLuaError(lua_ret);

  // Look for the behaviour's signature. This is a table pushed onto the stack.
  if (!lua_istable(lua_state, -1))
    throw std::runtime_error("Could not find signature for " + file_name);

  // Found a signature - extract the table's fields
  behaviour_signature signature;

  // Add a static assert to alert the user (at compile time) if the number of
  // fields in the behaviour signature has changed in the header file but not
  // here. This won't protect against the case where the number of fields has
  // stayed the same, but the types and/or the names have changed.
  static_assert(
      std::extent<decltype(behaviour_signature_fields)>::value == 3,
      "The number of signature fields does not match processing below");

  for (std::string const& field : behaviour_signature_fields) {
    lua_pushstring(lua_state, field.c_str());
    lua_gettable(lua_state, -2);  // retrieve table[field]

    if ((field == "name") && (lua_isstring(lua_state, -1)))
      signature.name = lua_tostring(lua_state, -1);
    else if ((field == "description") && (lua_isstring(lua_state, -1)))
      signature.description = lua_tostring(lua_state, -1);
    else if ((field == "entry_point") && (lua_isfunction(lua_state, -1)))
      signature.entry_point_ref = luaL_ref(lua_state, LUA_REGISTRYINDEX);
    else
      throw std::runtime_error("Signature table incorrect for " + file_name);

    lua_pop(lua_state, 1);
  }

  bool success = false;

  // Add the behaviour to the index
  std::tie(std::ignore, success) =
      behaviour_index.insert({signature.name, signature});

  if (!success)
    throw std::runtime_error(signature.name + " already added - skipping");

  // Cache the behaviour's path
  std::tie(std::ignore, success) =
      behaviour_path.insert({signature.name, path.parent_path()});

  if (!success)
    throw std::runtime_error("Unable to cache path for " + signature.name);
}

// Process the arguments passed to the behaviours. Arguments are passed as
// 'key=value' pairs on the command line. This function processes these pairs
// and creates a Lua table (which is accessed through the registry). Note
// that all behaviours receive the whole table. Three types of value are
// supported. Numeric types are passed using lua_pushnumber, boolean types
// are passed with lua_pushboolean and everything else is passed as a string.
void LuaManager::ProcessArguments(std::vector<std::string> const& arguments) {
  // Create a hash-table and populate it using regular expressions. Note that
  // the 'narr' parameter is for when Lua tables are used as vectors.
  lua_createtable(lua_state, 0, arguments.size());

  std::regex integer_regex(R"((\w+)=([-+]?[0-9]+)$)");
  std::regex number_regex(R"((\w+)=([-+]?[0-9]+\.[0-9]*)$)");
  std::regex bool_regex(R"((\w+)=(true|false))", std::regex_constants::icase);
  std::regex string_regex(R"((\w+)=([[:graph:]]+))");

  for (auto const& arg : arguments) {
    std::smatch match;

    try {
      if (std::regex_search(arg, match, integer_regex))
        lua_pushinteger(lua_state, std::stoi(match[2].str()));
      else if (std::regex_search(arg, match, number_regex))
        lua_pushnumber(lua_state, std::stod(match[2].str()));
      else if (std::regex_search(arg, match, bool_regex))
        lua_pushboolean(lua_state, !strcasecmp(match[2].str().c_str(), "true"));
      else if (std::regex_search(arg, match, string_regex))
        lua_pushstring(lua_state, match[2].str().c_str());
      else
        throw std::runtime_error("Could not parse Lua argument: " + arg);
    } catch (std::logic_error const& e) {
      throw std::runtime_error("Exception converting Lua argument " + arg +
                               ": " + e.what());
    }

    lua_setfield(lua_state, -2, match[1].str().c_str());

    log_debug("Successfully processed Lua argument {}", arg.c_str());
  }

  // Store a reference to the argument table for when the behaviours are run
  lua_ref_argument_table = luaL_ref(lua_state, LUA_REGISTRYINDEX);
  if (lua_ref_argument_table == LUA_REFNIL)
    throw std::runtime_error("Could not set reference to Lua argument table");
}

// Runs the loaded behaviour
void LuaManager::RunBehaviour(void) {

  // Lookup the behaviour's signature
  auto index_it = behaviour_index.begin();
  if (index_it == behaviour_index.end())
    throw std::runtime_error("Behaviour is not loaded");

  auto behaviour_name = index_it->second.name;

  // Lookup the reference and check that it points to a function
  int type_of_ref = lua_rawgeti(
      lua_state, LUA_REGISTRYINDEX, index_it->second.entry_point_ref);
  if (type_of_ref != LUA_TFUNCTION)
    throw std::runtime_error("Lua reference does not point to a function");

  // Retrieve the behaviour's path
  auto path_it = behaviour_path.find(behaviour_name);
  if (path_it == behaviour_path.end())
    throw std::runtime_error("Can not lookup path for " + behaviour_name);

  // Push the argument table onto the stack (if necessary)
  if (lua_ref_argument_table != LUA_NOREF) {
    int type_of_ref =
        lua_rawgeti(lua_state, LUA_REGISTRYINDEX, lua_ref_argument_table);
    if (type_of_ref != LUA_TTABLE)
      throw std::runtime_error("Lua reference does not point to a table");
  }

  // Setup the package path so that the user can 'require' files in the same
  // directory as the behaviour
  SetLuaPackagePath(path_it->second);

  // Call the entry point for the behaviour (this may not return). Note that
  // there is only ever either 0 or 1 arguments passed to the behaviour.
  int lua_ret =
      lua_pcall(lua_state, lua_ref_argument_table != LUA_NOREF, LUA_MULTRET, 0);
  if (lua_ret != LUA_OK)
    ProcessLuaError(lua_ret);
}

// Process error values returned by Lua calls
void LuaManager::ProcessLuaError(int lua_ret) {
  std::string error_message = lua_tostring(lua_state, -1);

  switch (lua_ret) {
    case LUA_ERRSYNTAX:
      throw std::runtime_error("syntax error: " + error_message);
    case LUA_ERRMEM:
      throw std::runtime_error("out-of-memory: " + error_message);
    case LUA_ERRRUN:
      throw std::runtime_error("runtime error: " + error_message);
    case LUA_ERRERR:
      throw std::runtime_error("error: " + error_message);
#if LUA_VERSION_NUM == 503
    case LUA_ERRGCMM:
      throw std::runtime_error("garbage collection: " + error_message);
#endif
  }
};

// Set the Lua Package Path so that Lua can find files that are 'required'
void LuaManager::SetLuaPackagePath(std::string const& path) {
  std::string package_path =
      "package.path = package.path .. ';" + path + "/?.lua'";

  int lua_ret = luaL_dostring(lua_state, package_path.c_str());
  if (lua_ret != LUA_OK)
    ProcessLuaError(lua_ret);

  log_trace("Set Lua package path: " + package_path);
}
