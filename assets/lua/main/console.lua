local world = World.get()

local logs = {}
Events.observe("consoleLog", function(data)
    local log = data:getString("log")
    table.insert(logs, log)
end)

local consoleInput = ""
Events.observe("render", function()
    if ImGui.beginWindow("Console") then
        local input = ImGui.inputText(consoleInput)
        consoleInput = input[1]

        ImGui.sameLine()
        if ImGui.button("Execute") or input[2] then
            Console.runCommand(consoleInput)
            consoleInput = ""
        end

        if ImGui.beginChild("Scrolling") then
            for _, log in ipairs(logs) do
                ImGui.text(log)
            end
        end
        ImGui.endChild()
    end
    ImGui.endWindow()
end)

Console.registerCommand("current_fps", function()
    Console.log("FPS : "..tostring(1 / EngineInfo.deltaTime))
end)

Console.registerCommand("current_gametime", function()
    Console.log("Gametime : "..tostring(EngineInfo.gameTime))
end)