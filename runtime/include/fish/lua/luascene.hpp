#pragma once

#include "fish/scenes.hpp"
#include <sol/state.hpp>
#include <sol/table.hpp>
namespace fish::lua
{
    struct LuaSceneModel {
        LuaSceneModel(Scene::Model& model, sol::state& state);

        Scene::Model& model;
        sol::table getVertices();
    private:
        sol::state& state;
    };

    struct LuaScene {
        LuaScene(const Scene& scene, SceneLoader& loader, sol::state& state);

        Scene scene;
        void loadIntoWorld();
        sol::table getModels(); // returns a list of LuaSceneModels
    private:
        SceneLoader& loader;
        sol::state& state;
    };
}