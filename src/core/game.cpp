
#include "game.h"

#include "entity.h"
#include "logger.h"
#include "platform.h"
#include "scene.h"
#include "scene_serializer.h"
#include "asset_manager.h"

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
