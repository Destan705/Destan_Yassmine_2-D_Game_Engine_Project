#include "phase3_demo_game.h"
#include "entity.h"
#include "scene.h"
#include "logger.h"
#include "behavior_component.h"
#include "keyboard_move.h"
#include "trigger_sound.h"
#include <memory>


void Phase3DemoGame::onStart(AudioService& audioService) {
    Phase3DemoGame::audioService = &audioService;
    testSoundId = audioService.loadWav("audio/test.wav");
    behaviors.registerBehavior("KeyboardMove", [] {
        return std::make_unique<KeyboardMove>(200.0f);
    });
}

void Phase3DemoGame::onSceneChanged(Scene& scene, const std::filesystem::path&) {
    Entity* player = nullptr;
    Entity* playerTrigger = nullptr;
    for(auto& entity:scene.getEntities()){
        if(entity->getName() == "Player" && player == nullptr){
            player = entity.get();
        }
        if (entity->getName() == "Player Trigger" && playerTrigger == nullptr) {
            playerTrigger = entity.get();
        }
        if (player != nullptr && playerTrigger != nullptr){
            break;
        }
    }
    if (player != nullptr){
        auto* behavior = player->getComponent<BehaviorComponent>();
        if(behavior == nullptr){
            behavior = &player->addComponent<BehaviorComponent>();
        }
        if (!behavior->getBehavior("KeyboardMove")) {
            behavior->addBehavior("KeyboardMove", behaviors.create("KeyboardMove"));
        }
    }

    if (player != nullptr && playerTrigger != nullptr && audioService != nullptr){
        auto* behavior = playerTrigger->getComponent<BehaviorComponent>();
        if(behavior == nullptr){
            behavior = &playerTrigger->addComponent<BehaviorComponent>();
        }
        if (!behavior->getBehavior("TriggerSound")) {
            behavior->addBehavior("TriggerSound", std::make_unique<TriggerSound>(*audioService, testSoundId, player->getId())); 
        }
    }
}