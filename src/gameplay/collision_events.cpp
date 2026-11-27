#include "collision_events.h"
#include "scene.h"
#include "entity.h"
#include "behavior_component.h"

#include <algorithm>
#include <utility>
#include <tuple>


void CollisionEventSystem::reset() {
    previousPairs.clear();
}

std::vector<CollisionEvent> CollisionEventSystem::update(const std::vector<ColliderSystem::CollisionPair>& collisionPairs){
    std::set<ColliderSystem::CollisionPair> currentPairs;
    std::vector<CollisionEvent> events;

    for (const auto &pair: collisionPairs){
        if (pair.first == pair.second){
            continue;
        }

        const std::uint64_t smallerId = std::min(pair.first, pair.second);
        const std::uint64_t largerId = std::max(pair.second, pair.first);

        currentPairs.insert({smallerId, largerId});
    }

    for (const auto &pair: currentPairs){
        CollisionPhase phase;
        if (previousPairs.find(pair) != previousPairs.end()) {
            phase = CollisionPhase::Stay;
        } else {
            phase = CollisionPhase::Enter;
        }
        events.emplace_back(pair.first, pair.second, phase);
    }

    for (const auto &pair: previousPairs){
        if (currentPairs.find(pair) == currentPairs.end()) {
            events.emplace_back(pair.first, pair.second, CollisionPhase::Exit);
        }
    }

    std::sort(events.begin(), events.end(), [](const auto& a, const auto& b){
        return std::tie(a.entityAId, a.entityBId) < std::tie(b.entityAId, b.entityBId);
    });

    previousPairs = std::move(currentPairs);

    return events;
}


void dispatchCollisionEvents(Scene& scene, const std::vector<CollisionEvent>& events) {

    auto dispatchEntityCollision = [&scene](const CollisionEvent& event, bool isA) {
        Entity* entity = isA ? scene.findEntity(event.entityAId) : scene.findEntity(event.entityBId);
        if (entity != nullptr){
            BehaviorComponent* behaviorComponent = entity->getComponent<BehaviorComponent>();
            if (behaviorComponent != nullptr) {
                behaviorComponent->onCollision(event); 
            }
        }
    };

    for (const auto &event: events){

        dispatchEntityCollision(event, true); // dispatch for entityA

        dispatchEntityCollision(event, false); // dispatch for entityB
    }
}