/**
 * bwt_mcm_lua_manager.hpp
 *
 * Definition of the MCM's Lua manager class.
 *
 * This header defines the interface to the MCM's Lua manager i.e. the software
 * agent that manages the interactions with the Lua interpreter plus loading
 * and running the different 'behaviours'.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "lua.hpp"

class LuaManager {
public:
  // Constructors
  LuaManager();
  LuaManager(std::string const& file_name);

  ~LuaManager();

  // Disable copy, move and assignment constructors
  LuaManager(LuaManager const& rhs) = delete;
  LuaManager(LuaManager&& rhs) = delete;
  LuaManager& operator=(LuaManager const& rhs) = delete;
  LuaManager& operator=(LuaManager&& rhs) = delete;

  // Load a behaviour from file
  void LoadBehaviour(std::string const& file_name);

  // Process the arguments passed to the behaviours. Arguments are passed as
  // 'key=value' pairs on the command line. This function processes these pairs
  // and creates a Lua table (which is accessed through the registry). Note
  // that all behaviours receive the whole table. Three types of value are
  // supported. Numeric types are passed using lua_pushnumber, boolean types
  // are passed with lua_pushboolean and everything else is passed as a string.
  void ProcessArguments(std::vector<std::string> const& arguments);

  // Runs the loaded behaviour
  void RunBehaviour();

private:
  // By default, the Lua run-time uses C to manager the memore for instances
  // of 'lua_State', so we can't use a smart pointer here. Instead, use RAII
  // to guarentee cleanup.
  inline static lua_State* lua_state = nullptr;

  // Define the default file extension for behaviours
  inline static const std::string behaviour_file_extension = ".lua";

  // Data structures to store a Lua behaviour's signature
  using behaviour_signature = struct {
    std::string name{};
    std::string description{};
    int entry_point_ref = LUA_NOREF;
  };

  inline static const std::string behaviour_signature_fields[] = {
      "name", "description", "entry_point"};

  // Declare a Lua reference to a table of arguments passed to the behaviours
  int lua_ref_argument_table = LUA_NOREF;

  // Provide a mapping between the name of a behaviour and its signature
  std::unordered_map<std::string, behaviour_signature> behaviour_index;

  // Provide a mapping between the name of a behaviour and its path
  std::unordered_map<std::string, std::string> behaviour_path;

  // Process error values returned by Lua calls
  void ProcessLuaError(int lua_ret);

  // Set the Lua Package Path so that Lua can find files that are 'required'
  void SetLuaPackagePath(std::string const& path);
};
