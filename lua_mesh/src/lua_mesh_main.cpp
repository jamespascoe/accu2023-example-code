/**
 * lua_mesh_main.cpp
 *
 * This file defines the entry point for LuaMesh. Command line arguments are
 * parsed using the excellent cxxopts (https://github.com/jarro2783/cxxopts/).
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */

#include <iostream>
#include <limits>

#include "cxxopts/cxxopts.hpp"

#include "lua_mesh_log_manager.hpp"
#include "lua_mesh_lua_manager.hpp"

// Parse command line arguments (see cxxopts.hpp for details)
static cxxopts::ParseResult parse(int argc, char *argv[])
{
  cxxopts::Options options(argv[0],
     "Lua Mesh - A Lua Based Tool for Simulating Mobile Mesh Networks");

  options.positional_help("[Lua behaviour to run]").show_positional_help();

  options.add_options()
    ("a,args", "Arguments passed to all Lua behaviours. Expressed in "
               "'key=value' form and can be used multiple times",
     cxxopts::value<std::vector<std::string>>())
    ("h,help", "Print help and exit")
    ;

  options.add_options("Logging")
    ("f,filename", "Log filename",
     cxxopts::value<std::string>()->default_value(LogManager::def_log_name))
    ("l,level", "Console logging level: "
        "off, trace, debug, info, warning, error and critical",
     cxxopts::value<std::string>()->default_value(LogManager::def_log_lvl))
    ("log-file-level", "Log file logging level: "
        "off, trace, debug, info, warning, error and critical",
     cxxopts::value<std::string>()->default_value(LogManager::def_file_lvl))
    ;

  options.add_options("hidden-group")("behaviour", "Lua behaviour to run",
                                      cxxopts::value<std::string>());

  try
  {
    options.parse_positional("behaviour");

    auto parse_result = options.parse(argc, argv);
    std::vector<std::string> help_groups_to_show{"", "Logging"};

    if (parse_result.count("help")) {
      std::cout << options.help(help_groups_to_show) << std::endl;
      std::exit(EXIT_SUCCESS);
    }

    if (parse_result["behaviour"].count() == 0) {
      std::cerr << "Please specify a Lua behaviour to run!" << std::endl;
      std::cerr << options.help(help_groups_to_show) << std::endl;
      std::exit(EXIT_FAILURE);
    }

    return parse_result;

  } catch (cxxopts::OptionException const& e) {
    std::cerr << "Error parsing command line: " << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  // Parse the command line arguments
  auto arguments = parse(argc, argv);

  try {
    // Initialise the logging manager
    LogManager::initialise(arguments["filename"].as<std::string>(),
                           arguments["level"].as<std::string>(),
                           arguments["log-file-level"].as<std::string>());
  } catch (std::runtime_error const &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // Instantiate the Lua behaviour manager
  LuaManager lua_m{arguments["behaviour"].as<std::string>()};

  // Pass in the Lua arguments (if specified)
  if (arguments.count("args")) {
    try {
      lua_m.ProcessArguments(arguments["args"].as<std::vector<std::string>>());
    } catch (std::runtime_error const& e) {
      std::cerr << e.what() << std::endl;
      return EXIT_FAILURE;
    }
  }

  // Run the behaviour
  try {
      lua_m.RunBehaviour();
  } catch (FatalExcepton const& e) {
    // This has been thrown by LogManager's log_fatal call(). The error has
    // already been printed so just exit
    return EXIT_FAILURE;
  } catch (std::runtime_error const& e) {
    std::cerr << "Error running Lua behaviour: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
