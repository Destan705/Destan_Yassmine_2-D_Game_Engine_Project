#pragma once

#include <vector>
#include <cstdint>
#include <utility>

class Scene; 

class ColliderSystem {
    
    public:
        using CollisionPair = std::pair<std::uint64_t, std::uint64_t>;

        std::vector<CollisionPair> detectCollisions(Scene& scene) const;
    
};