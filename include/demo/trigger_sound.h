#pragma once

#include "behavior.h"
#include "audio_service.h"

#include <cstdint>

class TriggerSound : public Behavior {
    
    public:
        TriggerSound(AudioService& audioService, AudioService::SoundId soundId, std::uint64_t targetEntityId);

        void onCollision(const CollisionEvent& event) override;

    private:
        AudioService& audioService;
        AudioService::SoundId soundId;
        std::uint64_t targetEntityId;
};