Console = {}

Console._visible = false

function Console.show()
    Console._visible = true
end

function Console.hide()
    Console._visible = false
end

local logs = {}
Events.observe("consoleLog", function(data)
    local log = data:getString("log")
    table.insert(logs, log)
end)

local consoleInput = ""
Events.observe("render", function()
    if not Console._visible then
        return
    end

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