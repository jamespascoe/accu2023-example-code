/**
 * lua_chat_action_log.hpp
 *
 * This file declares the interface for an action that allows Lua behaviours
 * to log messages via the LogManager
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */
#pragma once

#include <string>

/*
 * This class provides means to write messages at different severity levels
 * directly from the Lua behaviours
 */
class Log {
 public:
  Log() = delete;

  static void trace(std::string const& message);

  static void debug(std::string const& message);

  static void info(std::string const& message);

  static void warn(std::string const& message);

  static void error(std::string const& message);

  static void critical(std::string const& message);

  static void fatal(std::string const& message);
};
