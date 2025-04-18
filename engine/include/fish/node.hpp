#pragma once

#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/entt.hpp"
#include <cstddef>

namespace fish
{
    struct Node {
        const entt::entity me;

        size_t children;
        entt::entity parent = entt::null;

        entt::entity firstChild = entt::null;
        entt::entity previousParentChild = entt::null;
        entt::entity nextParentChild = entt::null;
    };
}