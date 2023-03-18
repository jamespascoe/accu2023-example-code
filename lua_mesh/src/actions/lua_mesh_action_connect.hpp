/**
 * lua_chat_action_scan.hpp
 *
 * This action allows Lua behaviours to 'connect' to other nodes that
 * are connected to the Mesh.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */
#pragma once

class Connect {
public:

  Connect();
  ~Connect();

  // Do not allow instances to be copied or moved
  Connect(Connect const& rhs) = delete;
  Connect(Connect&& rhs) = delete;
  Connect& operator=(Connect const& rhs) = delete;
  Connect& operator=(Connect&& rhs) = delete;

  // Connect the RF environment for other Mobile Mesh nodes
  void DoConnect();

private:
};
