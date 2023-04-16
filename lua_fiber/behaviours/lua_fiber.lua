--[[

 lua_fiber.lua

 This behaviour provides a simple example of fibers. The C++
 actions allow the behaviour to be arbitrarily complex, whilst
 mocking out the low-level interactions with the hardware.

 Copyright © Blu Wireless. All Rights Reserved.
 Licensed under the MIT license. See LICENSE file in the project.
 Feedback: james.pascoe@bluwireless.com

]]

function ping_fiber (connector, remote_port)

  Actions.Log.info(
    "ping_fiber: connecting to port: " .. remote_port
  )

  local timer = Actions.Timer()

  -- Connect to a node and send a 'ping' message
  while true do

    connector:Send("localhost", remote_port, "PING")

    repeat
      coroutine.yield()
    until (connector:IsMessageAvailable())

    Actions.Log.info(
      "ping_fiber: received: " .. connector:GetNextMessage()
    )

    timer(Actions.Timer.WaitType_NOBLOCK, 1, "s", 1)
    while timer:IsWaiting() do
      coroutine.yield()
    end

  end

end

function pong_fiber (connector, remote_port)

  Actions.Log.info(
    "pong_fiber: connecting to port: " .. remote_port
  )

  local timer = Actions.Timer()

  -- Connect to a node and send a 'pong' message
  while true do

    repeat
      coroutine.yield()
    until (connector:IsMessageAvailable())

    Actions.Log.info(
      "pong_fiber: received: " .. connector:GetNextMessage()
    )

    connector:Send("localhost", remote_port, "PONG")

    timer(Actions.Timer.WaitType_NOBLOCK, 1, "s", 2)
    while timer:IsWaiting() do
      coroutine.yield()
    end

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

    -- Run the dispatcher every 1 ms. Note, that a blocking timer is required
    -- to prevent lua_mesh from consuming 100% of a core.
    timer(Actions.Timer.WaitType_BLOCK, 1, "ms", 3)

  end
end

local function main(args)

  print("Welcome to Lua Fiber !")

  if (not args or not args["port"]) then
    print("Usage: lua_fiber lua_fiber.lua -a port=<listen port>")
    os.exit(1)
  end

  print(
    string.format("Starting Lua Fiber:\n" ..
                  "  listen port: %d\n", tonumber(args["port"]))
  )

  local connector = Actions.Connector(args["port"])
  local remote_port = args["port"] == 7777 and "8888" or "7777"

  -- Create co-routines
  local coroutines = {}
  coroutines["ping"] = coroutine.create(ping_fiber)
  coroutine.resume(coroutines["ping"], connector, remote_port)

  coroutines["pong"] = coroutine.create(pong_fiber)
  coroutine.resume(coroutines["pong"], connector, remote_port)

  -- Run the main loop
  dispatcher(coroutines)

end

local behaviour = {
  name = "lua_fiber",
  description = "A Lua behaviour to demonstrate fibers",
  entry_point = main
}

return behaviour
