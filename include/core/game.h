
#pragma once

#include "colliderSystem.h"

#include <memory>
#include <vector>

class Scene;

// Builds the initial game scene.
// SceneManager takes ownership of the returned Scene.
std::unique_ptr<Scene> createGameScene();

// Allows gameplay to react to overlapping Entity pairs.
// Detection alone does not block movement or apply physics.
void handleCollisions(
    const std::vector<ColliderSystem::CollisionPair>& collisionPairs
);
