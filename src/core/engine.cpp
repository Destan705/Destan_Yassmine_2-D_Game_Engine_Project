
#include "engine.h"

#include "audio_service.h"
#include "colliderSystem.h"
#include "event_dispatcher.h"
#include "game.h"
#include "input.h"
#include "logger.h"
#include "platform.h"
#include "scene_manager.h"

#include <chrono>
#include <exception>
#include <string>

void renderEngineFrame(SceneManager& sceneManager) {
    renderPlatform(sceneManager.getActiveScene());
}

void engine() {
    bool running = initializePlatform();

    if (!running) {
        logMessage(
            logger_level::Error,
            "Error occurred while initializing engine"
        );
        return;
    }

    try {
        initializeInput();

        float delta_time = 0.0f;

        auto prev_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count();

        {
            // Initialize engine systems.
            AudioService audioService(getAssetRootPath());
            EventDispatcher eventDispatcher;
            ColliderSystem colliderSystem;
            SceneManager sceneManager(eventDispatcher);

            // Load the test sound once.
            const auto testSoundId = audioService.loadWav("audio/test.wav");

            // Let gameplay create the Scene and its Entities.
            sceneManager.setActiveScene(createGameScene(getPlatformAssetManager()));

            
            bool wasOverlapping = false;    
            
            while (running) {
                auto current_time =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()
                    ).count();

                // Process input.
                running = processPlatformEvents();

                if (!running) {
                    break;
                }

                // Calculate delta time.
                delta_time = (current_time - prev_time) / 1000.0f;

                // Apply player movement from keyboard input
                updatePlayerMovement(sceneManager.getActiveScene(), delta_time);

                // Update the Scene and its Components.
                sceneManager.update(delta_time);

                // Detect collisions after position updates.
                auto collisionPairs = colliderSystem.detectCollisions(
                    sceneManager.getActiveScene()
                );

                // Let gameplay react to detected overlaps.
                bool justEntered = handleCollisions(collisionPairs, sceneManager.getActiveScene(), wasOverlapping);
                
                if (justEntered) {
                    logMessage(logger_level::Info, "Player entered trigger!");

                    if (testSoundId != 0 && !audioService.play(testSoundId)) {
                        logMessage(logger_level::Warning, "Failed to play trigger sound");
                    }
                }

                // Render the active Scene.
                renderEngineFrame(sceneManager);

                prev_time = current_time;
            }
        }

        // AudioService, SceneManager, and other systems
        // are destroyed before SDL shuts down.

    } catch (const std::exception& e) {
        logMessage(
            logger_level::Error,
            std::string("Engine error: ") + e.what()
        );
    } catch (...) {
        logMessage(
            logger_level::Error,
            "An unknown error occurred in the engine"
        );
    }

    // Runs after normal execution or a caught exception.
    shutdownPlatform();
}
