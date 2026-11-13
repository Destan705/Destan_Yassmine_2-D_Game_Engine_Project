
#include "engine.h"

#include "audio_service.h"
#include "collider_system.h"
#include "event_dispatcher.h"
#include "game.h"
#include "input.h"
#include "logger.h"
#include "platform.h"
#include "scene_manager.h"
#include "scene_serializer.h"

#ifdef ENGINE_EDITOR
#include "editor_state.h"
#endif

#include <chrono>
#include <exception>
#include <filesystem>
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
            SceneSerializer sceneSerializer;

            #ifdef ENGINE_EDITOR
           EditorState editorState;
            #endif


            // Save beside the demo assets, not in the working directory.
            const std::filesystem::path savedScenePath =
                getAssetRootPath() / "scenes/phase3-demo-saved.json";

            // Load the trigger sound once.
            const auto testSoundId = audioService.loadWav("audio/test.wav");

            // Load the original demo Scene from JSON.
            sceneManager.setActiveScene(
                createGameScene(getPlatformAssetManager())
            );

            #ifdef ENGINE_EDITOR
            // TEMP (P4-115): select the player for testing until click-selection exists.
            editorState.onSceneReplaced();
            
            #endif

            sceneManager.update(0.0f);


            // Remember whether the player was inside the trigger.
            bool wasOverlapping = false;

            while (running) {
                auto current_time =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()
                    ).count();

                // 1. Process input.
                running = processPlatformEvents();

                if (!running) {
                    break;
                }

                beginEditorFrame();

                // 2. Calculate delta time.
                delta_time = (current_time - prev_time) / 1000.0f;

                bool simulate = true;

                #ifdef ENGINE_EDITOR
                    
                simulate = !editorState.getPause();

                #endif

                if(simulate==true){

                // 3. Move player using keyboard input.
                updatePlayerMovement(
                    sceneManager.getActiveScene(),
                    delta_time
                );

                // 4. Update Scene and Components.
                sceneManager.update(delta_time);
            }

                // 5. Detect collisions.
                auto collisionPairs = colliderSystem.detectCollisions(
                    sceneManager.getActiveScene()
                );


                // 6. React to new trigger overlaps.
                bool justEntered = handleCollisions(
                     collisionPairs, sceneManager.getActiveScene(),
                    wasOverlapping
                );

                if (justEntered && simulate==true) {
                    logMessage(
                        logger_level::Info,
                        "Player entered trigger!"
                    );

                    if (testSoundId != 0 &&
                        !audioService.play(testSoundId)) {
                        logMessage(
                            logger_level::Warning,
                            "Failed to play trigger sound"
                        );
                    }
                }
                
                #ifdef ENGINE_EDITOR
                drawEditorPanels(sceneManager.getActiveScene(), editorState);
                #endif

                // 7. Render the current Scene.
                renderEngineFrame(sceneManager);       

                // 8. Handle save/reload at the frame boundary.

                // F5: Save the current Scene.
                if (wasKeyPressed(Key::F5)) {
                    try {
                        sceneSerializer.save(
                            sceneManager.getActiveScene(),
                            savedScenePath,
                            getPlatformAssetManager()
                        );

                        logMessage(
                            logger_level::Info,
                            "Scene saved to: " + savedScenePath.string()
                        );

                    } catch (const std::exception& e) {
                        logMessage(
                            logger_level::Error,
                            std::string("Failed to save Scene: ") + e.what()
                        );
                    }
                }

                // F9: Reload the saved Scene.
                if (wasKeyPressed(Key::F9)) {

                    try {
    
                        if (!std::filesystem::exists(savedScenePath)) {
                            logMessage(
                                logger_level::Warning,
                                "No saved Scene found. Press F5 first."
                            );
                        } else {
                            // Fully load the replacement before activating it.
                            auto newScene = sceneSerializer.load(
                                savedScenePath,
                                getPlatformAssetManager()
                            );

                            // Replace the Scene at the frame boundary.
                            sceneManager.setActiveScene(std::move(newScene));


                             #ifdef ENGINE_EDITOR

                             editorState.onSceneReplaced();


                            #endif

                            sceneManager.update(0.0f);

                            // Forget overlaps belonging to the old Scene.
                            wasOverlapping = false;

                            logMessage(
                                logger_level::Info,
                                "Scene reloaded successfully!"
                            );
                        }

                    } catch (const std::exception& e) {
                        // The existing Scene stays active if loading fails.
                        logMessage(
                            logger_level::Error,
                            std::string("Failed to reload Scene: ") + e.what()
                        );
                    }
                }


                // F1: toggle pause (works even while typing)
                #ifdef ENGINE_EDITOR
                if (wasShortcutPressed(Key::F1)) {
                    if(editorState.getPause()){

                        editorState.resume();
                    
                       

                    }else{

                        editorState.pause();


                    }
                }
                #endif

                prev_time = current_time;
            }
        }

        // Engine systems are destroyed before SDL shuts down.

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

    shutdownPlatform();
}
