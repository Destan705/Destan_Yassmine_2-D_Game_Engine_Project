
#include "game.h"

#include "animation_component.h"
#include "collider_component.h"
#include "entity.h"
#include "logger.h"
#include "platform.h"
#include "rendertypes.h"
#include "scene.h"
#include "sprite_component.h"
#include "transform_component.h"

#include <memory>
#include <string>
#include <vector>

std::unique_ptr<Scene> createGameScene() {
    auto scene = std::make_unique<Scene>("Current Scene");

    // First example Entity.
    Entity& sceneEntity = scene->createEntity();
    TransformComponent& transformComponent = sceneEntity.addComponent<TransformComponent>();
    textureHandle handle = loadPlatformTexture("assets/test.bmp");
    sceneEntity.addComponent<SpriteComponent>(handle);

    transformComponent.x = 100.0f;
    transformComponent.y = 150.0f;
    transformComponent.width = 50.0f;
    transformComponent.height = 50.0f;

    sceneEntity.addComponent<ColliderComponent>(
        0.0f, 0.0f, 50.0f, 50.0f
    );

    // Second example Entity.
    Entity& sceneEntity2 = scene->createEntity();
    TransformComponent& transformComponent2 = sceneEntity2.addComponent<TransformComponent>();
    textureHandle handle2 = loadPlatformTexture("assets/test.bmp");
    sceneEntity2.addComponent<SpriteComponent>(handle2);

    transformComponent2.x = 50.0f;
    transformComponent2.y = 150.0f;
    transformComponent2.width = 50.0f;
    transformComponent2.height = 50.0f;

    sceneEntity2.addComponent<ColliderComponent>(
        0.0f, 0.0f, 50.0f, 50.0f
    );

    // Third example Entity, with an animation.
    Entity& sceneEntity3 = scene->createEntity();
    TransformComponent& transformComponent3 = sceneEntity3.addComponent<TransformComponent>();
    textureHandle handle3 = loadPlatformTexture("assets/anim_sheet.bmp");
    sceneEntity3.addComponent<SpriteComponent>(handle3);

    transformComponent3.x = 300.0f;
    transformComponent3.y = 150.0f;
    transformComponent3.width = 32.0f;
    transformComponent3.height = 32.0f;

    const std::vector<Bounds> frames = {
        {0.0f, 0.0f, 32.0f, 32.0f},
        {32.0f, 0.0f, 32.0f, 32.0f},
        {64.0f, 0.0f, 32.0f, 32.0f},
        {96.0f, 0.0f, 32.0f, 32.0f},
    };

    sceneEntity3.addComponent<AnimationComponent>(
        frames, 0.25f, true
    );

    return scene;
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
