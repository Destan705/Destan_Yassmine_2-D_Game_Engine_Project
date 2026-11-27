#include "trigger_sound.h"
#include "collision_events.h"
#include "entity.h"
#include "logger.h"

TriggerSound::TriggerSound(AudioService& audioService, AudioService::SoundId soundId, std::uint64_t targetEntityId) : audioService(audioService), soundId(soundId), targetEntityId(targetEntityId) {}


void TriggerSound::onCollision(const CollisionEvent& event) {
    if (event.phase != CollisionPhase::Enter){
        return;
    }

    Entity* ownerEntity = getOwner();

    if (ownerEntity == nullptr){
        return;
    }

    bool matchesForward = event.entityAId  == targetEntityId && event.entityBId == ownerEntity->getId();
    bool matchesReverse = event.entityBId == targetEntityId && event.entityAId == ownerEntity->getId();

    if (!matchesForward && !matchesReverse) {
        return;
    }

    logMessage(logger_level::Info, "Player entered trigger!");

    if (soundId != 0){
        if (!audioService.play(soundId)){
            logMessage(logger_level::Warning, "Audio playback was unsuccessful");
        }
    }

}