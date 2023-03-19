--[[

 lua_mesh.lua

 This behaviour simulates a Mobile Mesh. In particular, the actions
 allow the connection management behaviour to be arbitraily complex,
 whilst mocking out the low-level interactions with the hardware.

 Copyright © Blu Wireless. All Rights Reserved.
 Licensed under the MIT license. See LICENSE file in the project.
 Feedback: james.pascoe@bluwireless.com

]]

function connector_fiber ()

  local connector = Actions.Connector()

  -- Scan for a Mesh node and connect to it. Whilst part of the mesh,
  -- L3 (IP) traffic can flow.
  while true do
    Actions.Log.warn("In connector")
    coroutine.yield()
  end

end

function event_monitor_fiber ()

  -- Monitor the status of the connection. If the node becomes
  -- disconnected then signal the connector coroutine.
  while true do
    Actions.Log.warn("In event_monitor")
    coroutine.yield()
  end

end

-- Coroutine dispatcher (see Section 9.4 of 'Programming in Lua')
function dispatcher (coroutines)

  local timer = Actions.Timer()

  while true do
    if next(coroutines) == nil then break end -- no more coroutines to run

    for name, co in pairs(coroutines) do
      local status, res = coroutine.resume(co)

      if res then -- coroutine has returned a result (i.e. finished)

        if type(res) == "string" then  -- runtime error
          Actions.Log.critical("Lua coroutine '" .. tostring(name) ..
                               "' has exited with runtime error " .. res)
        else
          Actions.Log.warn("Lua coroutine '" .. tostring(name) .. "' exited")
        end

        coroutines[name] = nil

        break
      end
    end

    -- Run the dispatcher every 1 ms. Note, that this is required to prevent
    -- LuaChat from consuming 100% of a core. Note also that the notification
    -- value is set to the maximum value of a uint32_t to prevent conflicts
    -- with user timer ids (i.e. user timers can start from 0 and count up).
    timer(Actions.Timer.WaitType_BLOCK, 1, "ms", 0xffffffff)

  end
end

local function main(args)

  print("Welcome to Lua Mesh !")

  if (args) then
    Actions.Log.info("Arguments passed to Lua:")
    for k,v in pairs(args) do
      Actions.Log.info(string.format("  %s %s", tostring(k), tostring(v)))
    end
  end

  -- Create co-routines
  local coroutines = {}
  coroutines["connector"] = coroutine.create(connector_fiber)
  coroutine.resume(coroutines["connector"])

  coroutines["event_monitor"] = coroutine.create(event_monitor_fiber)
  coroutine.resume(coroutines["event_monitor"])

  -- Run the main loop
  dispatcher(coroutines)

end

local behaviour = {
  name = "LuaMesh",
  description = "A Lua behaviour to simulate a Mobile Mesh",
  entry_point = main
}

return behaviour
