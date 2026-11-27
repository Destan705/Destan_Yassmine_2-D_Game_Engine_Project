#pragma once

#include "game_app.h"
#include "audio_service.h"
#include "behavior_registry.h"


class Phase3DemoGame: public GameApp {
    public:
        void onStart(AudioService& audioService) override;
        void onSceneChanged(Scene& scene, const std::filesystem::path& scenePath) override;
        

    private:
        AudioService::SoundId testSoundId = 0;
        AudioService* audioService = nullptr;
        BehaviorRegistry behaviors;
};