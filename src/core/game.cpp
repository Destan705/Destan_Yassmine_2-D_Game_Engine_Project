
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

void handleCollisions(
    const std::vector<ColliderSystem::CollisionPair>& collisionPairs
) {
    // Gameplay can decide what each overlap means.
    // For now, log pairs to verify that detection works.
    //
    // This does not stop movement, push Entities apart,
    // or apply physics.

    for (const auto& [entityAId, entityBId] : collisionPairs) {
        logMessage(
            logger_level::Info,
            "Collision detected between Entity " +
                std::to_string(entityAId) +
                " and Entity " +
                std::to_string(entityBId)
        );
    }
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
