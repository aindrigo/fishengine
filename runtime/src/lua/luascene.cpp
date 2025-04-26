#include "fish/lua/luascene.hpp"
#include "fish/scenes.hpp"
#include "fish/lua/luamath.hpp"
#include <sol/state.hpp>
#include <sol/table.hpp>

namespace fish::lua
{
    LuaSceneModel::LuaSceneModel(Scene::Model& model, sol::state& state)
        : model(model), state(state)
    {}

    sol::table LuaSceneModel::getVertices()
    {
        auto tbl = this->state.create_table(model.vertices.size());
        
        for (auto& vertex : model.vertices) {
            LuaVertex luaVertex(vertex);

            tbl.add(luaVertex);
        }

        return tbl;
    }

    LuaScene::LuaScene(const Scene& scene, SceneLoader& loader, sol::state& state)
        : scene(scene), state(state), loader(loader)
    {}

    void LuaScene::loadIntoWorld()
    {
        loader.loadIntoWorld(scene);
    }
    
    sol::table LuaScene::getModels()
    {
        auto tbl = this->state.create_table(scene.models.size());

        for (auto& model : scene.models) {
            LuaSceneModel luaModel(model, state);
            tbl.add(luaModel);
        }

        return tbl;
    }
}