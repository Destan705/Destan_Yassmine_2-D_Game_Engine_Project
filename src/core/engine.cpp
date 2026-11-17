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
#include "editor_actions.h"
#include "scene_file_panel.h"
#endif

#include <chrono>
#include <exception>
#include <filesystem>
#include <string>
#include <cstdint>
#include <utility>


void renderEngineFrame(SceneManager& sceneManager) {
    renderPlatform(sceneManager.getActiveScene());
}


#ifdef ENGINE_EDITOR

bool processSceneFileCommand(const SceneFileCommand& command, SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot) {

    SceneRequestResult result{SceneRequestStatus::Rejected, "Unknown scene file command"};

    switch (command.type) {

        case SceneFileCommandType::None:
            return false;

        case SceneFileCommandType::Open:
            result = requestOpen(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, command.path);
            break;

        case SceneFileCommandType::Save: {
            SceneFileResult saveResult = saveScene(sceneManager, sceneSerializer, assetManager, editorState, assetRoot);
            setSceneFileFeedback(saveResult.message);
            return false;
        }

        case SceneFileCommandType::SaveAs: {
            SceneFileResult saveResult = saveSceneAs(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, command.path);
            setSceneFileFeedback(saveResult.message);
            return false;
        }

        case SceneFileCommandType::ConfirmSave:
            result = resolveUnsavedChanges(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, UnsavedChangesChoice::Save);
            break;

        case SceneFileCommandType::ConfirmDiscard:
            result = resolveUnsavedChanges(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, UnsavedChangesChoice::Discard);
            break;

        case SceneFileCommandType::ConfirmCancel:
            result = resolveUnsavedChanges(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, UnsavedChangesChoice::Cancel);
            break;

        case SceneFileCommandType::ProvideSavePath:
            result = provideSavePath(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, command.path);
            break;
    }

    setSceneFileFeedback(result.message);

    return result.status == SceneRequestStatus::ReadyToClose;
}

#endif


void engine() {

    bool running = initializePlatform();

    if (!running) {
        logMessage(logger_level::Error, "Error occurred while initializing engine");
        return;
    }

    try {

        initializeInput();

        float delta_time = 0.0f;

        auto prev_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

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
            #ifndef ENGINE_EDITOR

            const std::filesystem::path savedScenePath = getAssetRootPath() / "scenes/phase3-demo-saved.json";

            #endif


            // Load the trigger sound once.
            const auto testSoundId = audioService.loadWav("audio/test.wav");

            // Load the original demo Scene from JSON.
            sceneManager.setActiveScene(createGameScene(getPlatformAssetManager()));

            #ifdef ENGINE_EDITOR

            editorState.onSceneReplaced();
            editorState.setCurrentScenePath("scenes/phase3-demo.json");
            editorState.clearDirty();

            #endif

            sceneManager.update(0.0f);

            // Remember whether the player was inside the trigger.
            bool wasOverlapping = false;


            while (running) {

                auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();


                // 1. Process input and handle window-close requests.
                bool keepRunning = processPlatformEvents();

                if (!keepRunning) {

                    #ifdef ENGINE_EDITOR

                    if (editorState.getPendingAction() == PendingSceneAction::None && !isStandaloneSaveRequested()) {

                        SceneRequestResult closeResult = requestClose(editorState);

                        setSceneFileFeedback(closeResult.message);

                        if (closeResult.status == SceneRequestStatus::ReadyToClose) {
                            break;
                        }
                    }

                    #else

                    break;

                    #endif
                }

                beginEditorFrame();


                // 2. Calculate delta time.
                delta_time = (current_time - prev_time) / 1000.0f;

                bool simulate = true;

                #ifdef ENGINE_EDITOR

                simulate = !editorState.getPause();

                #endif


                if (simulate) {

                    // 3. Move player using keyboard input.
                    updatePlayerMovement(sceneManager.getActiveScene(), delta_time);

                    // 4. Update Scene and Components.
                    sceneManager.update(delta_time);
                }


                // Draw editor panels before detecting collisions so
                // Inspector edits are reflected in the collision snapshot.
                #ifdef ENGINE_EDITOR

                SceneFileCommand sceneFileCommand = drawEditorPanels(sceneManager.getActiveScene(), editorState);

                #endif


                // 5. Detect collisions.
                auto collisionPairs = colliderSystem.detectCollisions(sceneManager.getActiveScene());

                #ifdef ENGINE_EDITOR

                editorState.setCollisions(collisionPairs);

                #endif


                // 6. React to new trigger overlaps.
                bool justEntered = handleCollisions(collisionPairs, sceneManager.getActiveScene(), wasOverlapping);

                if (justEntered && simulate) {

                    logMessage(logger_level::Info, "Player entered trigger!");

                    if (testSoundId != 0 && !audioService.play(testSoundId)) {
                        logMessage(logger_level::Warning, "Failed to play trigger sound");
                    }
                }


                // 7. Render the current Scene.
                #ifdef ENGINE_EDITOR

                // Render the Scene, collider overlay and editor.
                renderPlatform(sceneManager.getActiveScene(), editorState);

                // Process scene file commands at the frame boundary.
                std::uint64_t previousGeneration = editorState.getSceneGeneration();

                if (processSceneFileCommand(sceneFileCommand, sceneManager, sceneSerializer, getPlatformAssetManager(), editorState, getAssetRootPath())) {
                    break;
                }

                // Forget overlaps belonging to the previous Scene.
                if (previousGeneration != editorState.getSceneGeneration()) {
                    wasOverlapping = false;
                }

                #else

                renderEngineFrame(sceneManager);

                #endif


                // 8. Handle legacy save/reload shortcuts in non-editor builds.
                #ifndef ENGINE_EDITOR

                // F5: Save the current Scene.
                if (wasKeyPressed(Key::F5)) {

                    try {

                        sceneSerializer.save(sceneManager.getActiveScene(), savedScenePath, getPlatformAssetManager());

                        logMessage(logger_level::Info, "Scene saved to: " + savedScenePath.string());

                    } catch (const std::exception& e) {

                        logMessage(logger_level::Error, std::string("Failed to save Scene: ") + e.what());
                    }
                }


                // F9: Reload the saved Scene.
                if (wasKeyPressed(Key::F9)) {

                    try {

                        if (!std::filesystem::exists(savedScenePath)) {

                            logMessage(logger_level::Warning, "No saved Scene found. Press F5 first.");

                        } else {

                            auto newScene = sceneSerializer.load(savedScenePath, getPlatformAssetManager());

                            sceneManager.setActiveScene(std::move(newScene));
                            sceneManager.update(0.0f);

                            wasOverlapping = false;

                            logMessage(logger_level::Info, "Scene reloaded successfully!");
                        }

                    } catch (const std::exception& e) {

                        logMessage(logger_level::Error, std::string("Failed to reload Scene: ") + e.what());
                    }
                }

                #endif


                #ifdef ENGINE_EDITOR

                // F1: Toggle pause only when no scene file operation is pending.
                if (wasShortcutPressed(Key::F1) && editorState.getPendingAction() == PendingSceneAction::None && !isStandaloneSaveRequested()) {

                    if (editorState.getPause()) {
                        editorState.resume();
                    } else {
                        editorState.pause();
                    }
                }

                #endif


                prev_time = current_time;
            }
        }

        // Engine systems are destroyed before SDL shuts down.

    } catch (const std::exception& e) {

        logMessage(logger_level::Error, std::string("Engine error: ") + e.what());

    } catch (...) {

        logMessage(logger_level::Error, "An unknown error occurred in the engine");
    }

    shutdownPlatform();
}