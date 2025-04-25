local world = World.get()

local input = ""
Events.observe("render", function()
    if ImGui.beginWindow("Console") then
        input = ImGui.textInput(input, "Command")

        if ImGui.button("Run") then
            Console.runCommand(input)
        end
    end
    ImGui.endWindow()

end)