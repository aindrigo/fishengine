#include "fish/lua.hpp"
#include "imgui.h"
#include "imgui_stdlib.h"
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

        imguiLua["textInput"] = [](std::string currentText, sol::optional<std::string> placeholder) {
            std::string* ptr = new std::string(currentText);

            ImGui::InputText(placeholder.value_or("").c_str(), ptr);

            std::string result = *ptr;
            delete ptr;
            
            return result;
        };

        imguiLua["button"] = [](std::string text) {
            return ImGui::Button(text.c_str());
        };

        imguiLua["endWindow"] = [](std::string name) {
            ImGui::End();
        };
    }
}