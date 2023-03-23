/**
 * lua_chat_action_log.cpp
 *
 * This file implements an action that allows Lua behaviours to log messages
 * via the LogManager.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#include "lua_fiber_log_manager.hpp"

#include "lua_fiber_action_log.hpp"

void Log::trace(std::string const& message) { log_trace(message); }

void Log::debug(std::string const& message) { log_debug(message); }

void Log::info(std::string const& message) { log_info(message); }

void Log::warn(std::string const& message) { log_warn(message); }

void Log::error(std::string const& message) { log_error(message); }

void Log::critical(std::string const& message) { log_critical(message); }

void Log::fatal(std::string const& message) { log_fatal(message); }
