
#pragma once

#include "collider_system.h"

#include <memory>
#include <vector>

class Scene;
class AssetManager;

// Allows gameplay to react to overlapping Entity pairs.
// Detection alone does not block movement or apply physics.
bool handleCollisions(const std::vector<ColliderSystem::CollisionPair>& collisionPairs, Scene& scene, bool& wasOverlapping);


void updatePlayerMovement(Scene& scene, float deltaTime);