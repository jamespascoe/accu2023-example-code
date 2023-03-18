/**
 * lua_chat_log_manager.cpp
 *
 * This file establishes logging facilities (using spdlog) throughout the code.
 * Note that access to the logging is provided in the Lua space through the
 * log action (see actions/lua_chat_action_log.hpp for details).
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#include <filesystem>

#include "lua_mesh_log_manager.hpp"

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

void LogManager::initialise(std::string const& log_file_spec,
                            std::string const& log_level,
                            std::string const& log_file_level) {
  // Create two logging sinks:
  //
  //    1. stderr - for displaying errors and warnings to the user
  //    2. a rotating file sink - for logging all messages in a manner that
  //       means the log files do not become large and unwieldy
  //
  // These are then combined into a single logger object which becomes the
  // default system logger. This means that the remainder of the code can
  // call the individual log level functions (e.g. log_trace or log_error)
  // without requiring explicit access to an instance of the logger.
  auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  stderr_sink->set_level(spdlog::level::from_str(log_level));

  // Create any required directories for the file logger
  namespace fs = std::filesystem;

  fs::path path = fs::path(log_file_spec).remove_filename();
  try {
    if (!path.empty())
      fs::create_directories(path);
  } catch (fs::filesystem_error const& e) {
    throw std::runtime_error("Could not create log file path (" +
                             path.string() + "): " + e.what());
  }

  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      log_file_spec, max_file_size, max_num_files, true  // new file on startup
  );
  file_sink->set_level(spdlog::level::from_str(log_file_level));

  spdlog::logger logger(logger_name, {stderr_sink, file_sink});

  // Flush on 'info' and above
  logger.flush_on(spdlog::level::info);

  spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

  // Enable all logging levels
  spdlog::set_level(spdlog::level::trace);
}
