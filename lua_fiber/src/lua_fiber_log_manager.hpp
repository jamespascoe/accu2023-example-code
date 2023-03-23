/**
 * lua_chat_log_manager.hpp
 *
 * This file declares an interface to the 'LogManager', which is an object
 * that provides logging facilities (using spdlog) throughout the code.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#pragma once

#include <string>

#include "spdlog/spdlog.h"

// Declare an exception to differentiate between calls to 'log_fatal' and
// runtime errors.
class FatalExcepton : public std::runtime_error {
public:
  FatalExcepton(std::string const& what) : std::runtime_error(what){};
};

class LogManager {

public:
  // This will throw on error
  static void initialise(std::string const& log_file_spec,
                         std::string const& log_level,
                         std::string const& log_file_level);

  // Default logging constants
  inline static const std::string def_log_lvl = "warning";
  inline static const std::string def_file_lvl = "info";
  inline static const std::string def_log_name = "logs/lua-fiber.log";

private:

  LogManager() = default;
  ~LogManager() = default;

  // Do not allow the LogManager to be copied or moved.
  LogManager(LogManager const& rhs) = delete;
  LogManager(LogManager&& rhs) = delete;
  LogManager& operator=(LogManager const& rhs) = delete;
  LogManager& operator=(LogManager&& rhs) = delete;

  // Constants relating to the logger.
  inline static const std::string logger_name = "LUA-FIBER";

  // Constants relating to the rotating file sink. Note that max_file_size
  // is expressed in bytes files are rotated when they reach 1 MB in size.
  inline static const int max_file_size = 1024 * 1024;
  inline static const int max_num_files = 50;
};

// Wrap spdlog's logging primitives into generically named functions
template <typename... Args>
void log_trace(std::string const& format, Args const&... args) {
  spdlog::trace(format, args...);
}

template <typename... Args>
void log_debug(std::string const& format, Args const&... args) {
  spdlog::debug(format, args...);
}

template <typename... Args>
void log_info(std::string const& format, Args const&... args) {
  spdlog::info(format, args...);
}

template <typename... Args>
void log_warn(std::string const& format, Args const&... args) {
  spdlog::warn(format, args...);
}

template <typename... Args>
void log_error(std::string const& format, Args const&... args) {
  spdlog::error(format, args...);
}

template <typename... Args>
void log_critical(std::string const& format, Args const&... args) {
  spdlog::critical(format, args...);
}

// Add a function to signal a fatal error (and exit)
template <typename... Args>
void log_fatal(std::string const& format, Args const&... args) {
  auto error = fmt::format(format, args...);

  spdlog::critical(error);
  spdlog::critical("FATAL ERROR - EXITING");

  throw FatalExcepton(error);
}
