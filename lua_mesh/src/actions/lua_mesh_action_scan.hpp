/**
 * lua_chat_action_Scan.hpp
 *
 * This action allows Lua behaviours to 'Scan' to other nodes that
 * are Scaned to the Mesh.
 *
 * Copyright © Blu Wireless. All Rights Reserved.
 * Licensed under the MIT license. See LICENSE file in the project.
 * Feedback: james.pascoe@bluwireless.com
 */
#pragma once

class Scan {
public:

  Scan();
  ~Scan();

  // Do not allow instances to be copied or moved
  Scan(Scan const& rhs) = delete;
  Scan(Scan&& rhs) = delete;
  Scan& operator=(Scan const& rhs) = delete;
  Scan& operator=(Scan&& rhs) = delete;

  // Scan the RF environment for other Mobile Mesh nodes
  void DoScan();

private:
};
