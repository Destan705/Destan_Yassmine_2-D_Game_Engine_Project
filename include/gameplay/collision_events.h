#pragma once

#include "collider_system.h"
#include <cstdint>
#include <set>
#include <vector>

class Scene;

enum class CollisionPhase {
    Enter,
    Stay,
    Exit
};

struct CollisionEvent {
    std::uint64_t entityAId;
    std::uint64_t entityBId;
    CollisionPhase phase;
};

class CollisionEventSystem {

    public:
        std::vector<CollisionEvent> update(const std::vector<ColliderSystem::CollisionPair>& collisionPairs);

        void reset();

    private:
        std::set<ColliderSystem::CollisionPair> previousPairs;
};

void dispatchCollisionEvents(Scene& scene, const std::vector<CollisionEvent>& events);