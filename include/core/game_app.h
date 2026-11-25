#pragma once

#include <vector>
#include <filesystem>
#include "collider_system.h"

class AudioService;
class Scene;

class GameApp {
    public:
        virtual ~GameApp() = default;
        virtual void onStart(AudioService&) {}
        virtual void onUpdate(Scene&, float) {}
        virtual void onCollisions(const std::vector<ColliderSystem::CollisionPair>&, Scene&, AudioService&, bool) {}
        virtual void onSceneChanged(Scene&, const std::filesystem::path&) {}
};