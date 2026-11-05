
#include "game.h"

#include "entity.h"
#include "logger.h"
#include "platform.h"
#include "scene.h"
#include "scene_serializer.h"
#include "asset_manager.h"
#include "animationComponent.h"
#include "transform.h"
#include "input.h"

#include <memory>
#include <string>
#include <vector>

std::unique_ptr<Scene> createGameScene(AssetManager& assetManager) {
    
    std::filesystem::path assetRoot = getAssetRootPath();

    std::filesystem::path demoPath = assetRoot / "scenes/phase3-demo.json";

    SceneSerializer sceneSerializer;

    return sceneSerializer.load(demoPath, assetManager);

}

bool handleCollisions(const std::vector<ColliderSystem::CollisionPair>& collisionPairs, Scene& scene, bool& wasOverlapping) {
    const auto& entities = scene.getEntities();

    // The demo's trigger is the second Entity.
    if (entities.size() < 2) {
        wasOverlapping = false;
        return false;
    }

    Entity* player = nullptr;

    // Find the player by its AnimationComponent.
    for (const auto& entity : entities) {
        if (entity->getComponent<AnimationComponent>() != nullptr) {
            player = entity.get();
            break;
        }
    }

    // No player means no valid trigger overlap.
    if (player == nullptr) {
        wasOverlapping = false;
        return false;
    }

    Entity* trigger = entities[1].get();

    if (player == trigger) {
        wasOverlapping = false;
        return false;
    }

    bool currentlyOverlapping = false;

    // Find the collision between our player and trigger.
    for (const auto& [entityAId, entityBId] : collisionPairs) {
        if ((entityAId == player->getId() && entityBId == trigger->getId()) || (entityAId == trigger->getId() && entityBId == player->getId())) {
            currentlyOverlapping = true;
            break;
        }
    }

    // A new entry happens only if they weren't overlapping before.
    bool justEntered = currentlyOverlapping && !wasOverlapping;

    // Remember the current state for the next frame.
    wasOverlapping = currentlyOverlapping;

    return justEntered;
}


void updatePlayerMovement(Scene& scene, float deltaTime) {
    constexpr float playerSpeed = 200.0f; // Pixels per second

    for (const auto& entity : scene.getEntities()) {
        // Find the player (the animated Entity).
        auto* animationComponent = entity->getComponent<AnimationComponent>();

        if (animationComponent == nullptr) {
            continue;
        }

        Transform* transformComponent = entity->getComponent<Transform>();

        if (transformComponent == nullptr) {
            return;
        }

        float movement = playerSpeed * deltaTime;

        // Left
        if (isKeyHeld(Key::A)) {
            transformComponent->x -= movement;
        }

        // Right
        if (isKeyHeld(Key::D)) {
            transformComponent->x += movement;
        }

        // Up
        if (isKeyHeld(Key::W)) {
            transformComponent->y -= movement;
        }

        // Down
        if (isKeyHeld(Key::S)) {
            transformComponent->y += movement;
        }

        break; // Only move the first animated Entity.
    }
}
