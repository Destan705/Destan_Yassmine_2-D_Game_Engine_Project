#pragma once

#include <vector>
#include <cstdint>
#include <utility>
#include <cstddef>


class Scene; 

class ColliderSystem {
    
    public:
        using CollisionPair = std::pair<std::uint64_t, std::uint64_t>;

        std::vector<CollisionPair> detectCollisions(Scene& scene) const;

        std::size_t getValidColliderCount(Scene& scene) const;
};