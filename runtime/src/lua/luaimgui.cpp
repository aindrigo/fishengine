#include "fish/lua.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
#include <sol/forward.hpp>
#include <sol/optional_implementation.hpp>
#include <sol/string_view.hpp>
#include <sol/table.hpp>
#include <string>
namespace fish
{
    void LuaService::initImGui()
    {
        auto imguiLua = this->luaState["ImGui"].get_or_create<sol::table>();

        imguiLua["beginWindow"] = [](std::string name) {
            return ImGui::Begin(name.c_str());
        };

        imguiLua["text"] = [](std::string text) {
            ImGui::Text("%s", text.c_str());
        };

        imguiLua["inputText"] = [&](std::string currentText, sol::optional<std::string> placeholder) {
            std::string* ptr = new std::string(currentText);
            
            ImGui::InputText(placeholder.value_or("##").c_str(), ptr);
            std::string result = *ptr;
            
            auto table = luaState.create_table(2);
            table.add(result);
            table.add(ImGui::IsItemDeactivatedAfterEdit());

            delete ptr;

            return table;
        };

        imguiLua["button"] = [](std::string text) {
            return ImGui::Button(text.c_str());
        };

        imguiLua["sameLine"] = []() {
            ImGui::SameLine();
        };

        imguiLua["beginChild"] = [](std::string id) {
            return ImGui::BeginChild(id.c_str());
        };

        imguiLua["endChild"] = []() {
            ImGui::EndChild();
        };

        imguiLua["endWindow"] = [](std::string name) {
            ImGui::End();
        };
    }
}