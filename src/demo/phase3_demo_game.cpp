#include "phase3_demo_game.h"
#include "entity.h"
#include "scene.h"
#include "logger.h"
#include "behavior_component.h"
#include "keyboard_move.h"
#include <memory>


void Phase3DemoGame::onStart(AudioService& audioService) {
    testSoundId = audioService.loadWav("audio/test.wav");
    wasOverlapping = false;
    behaviors.registerBehavior("KeyboardMove", [] {
    return std::make_unique<KeyboardMove>(200.0f);
});
}

bool Phase3DemoGame::handleCollisions(const std::vector<ColliderSystem::CollisionPair>& collisionPairs, Scene& scene) {
    const auto& entities = scene.getEntities();
    Entity* player = nullptr;
    Entity* trigger = nullptr;

    for (const auto& entity : entities) { 
        if (entity->getName() == "Player" && player == nullptr) {
            player = entity.get();
        } else if (entity->getName() == "Player Trigger" && trigger == nullptr){
            trigger = entity.get();
        }

        if (player != nullptr && trigger != nullptr ){
            break;
        }
    }

    if (player == trigger || player == nullptr || trigger == nullptr) {
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

void Phase3DemoGame::onCollisions(const std::vector<ColliderSystem::CollisionPair>& collisionPairs, Scene& scene, AudioService& audioService, bool simulate) {
    bool justEntered = handleCollisions(collisionPairs, scene);

    if (justEntered && simulate){
        logMessage(logger_level::Info, "Player entered trigger!");
        if (testSoundId != 0){
            if (!audioService.play(testSoundId)) {
                logMessage(logger_level::Warning, "Sound failed to play");
            }
        }
    }
}

void Phase3DemoGame::onSceneChanged(Scene& scene, const std::filesystem::path&) {
    wasOverlapping = false;
    for(auto& entity:scene.getEntities()){
        if(entity->getName() != "Player"){
            continue;
        }
        auto* behavior = entity->getComponent<BehaviorComponent>();
        if(behavior!=nullptr){
            return;
        }
        auto& behaviorComponent = entity->addComponent<BehaviorComponent>();
        behaviorComponent.addBehavior("KeyboardMove", behaviors.create("KeyboardMove"));
        return;
    }

}