
#include "engine.h"

#include "colliderSystem.h"
#include "event_dispatcher.h"
#include "game.h"
#include "input.h"
#include "logger.h"
#include "platform.h"
#include "scene_manager.h"

#include <chrono>

void renderEngineFrame(SceneManager& sceneManager) {
    renderPlatform(sceneManager.getActiveScene());
}

void engine() {
    bool running = initializePlatform();

    if (running) {
        initializeInput();
    }

    if (!running) {
        logMessage(
            logger_level::Error,
            "Error occurred while initializing engine"
        );
        return;
    }

    float delta_time = 0.0f;

    auto prev_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count();

    {
        EventDispatcher eventDispatcher;
        ColliderSystem colliderSystem;
        SceneManager sceneManager(eventDispatcher);

        // Let gameplay create the Scene and its Entities.
        sceneManager.setActiveScene(createGameScene());

        while (running) {
            auto current_time =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count();

            // Process input.
            running = processPlatformEvents();

            if (!running) {
                break;
            }

            // Calculate delta time.
            delta_time = (current_time - prev_time) / 1000.0f;

            // Update the Scene and its Components.
            sceneManager.update(delta_time);

            // Detect collisions after position updates.
            auto collisionPairs = colliderSystem.detectCollisions(
                sceneManager.getActiveScene()
            );

            // Let gameplay react to detected overlaps.
            handleCollisions(collisionPairs);

            // Render the active Scene.
            renderEngineFrame(sceneManager);

            prev_time = current_time;
        }
    }

    // Shutdown after SceneManager and systems are destroyed.
    shutdownPlatform();
}
