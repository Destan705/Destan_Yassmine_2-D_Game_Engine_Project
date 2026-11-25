#include "engine.h"

#include "audio_service.h"
#include "collider_system.h"
#include "event_dispatcher.h"
#include "game_app.h"
#include "input.h"
#include "logger.h"
#include "platform.h"
#include "scene_manager.h"
#include "scene_serializer.h"
#include "project_config.h"
#include "scene_path.h"

#ifdef ENGINE_EDITOR
#include "asset_manager.h"
#include "editor_actions.h"
#include "editor_state.h"
#include "engine_stats.h"
#include "scene_file_panel.h"
#endif

#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <string>
#include <utility>

void renderEngineFrame(SceneManager& sceneManager) {
    renderPlatform(sceneManager.getActiveScene());
}

#ifdef ENGINE_EDITOR
bool processSceneFileCommand(
    const SceneFileCommand& command,
    SceneManager& sceneManager,
    SceneSerializer& sceneSerializer,
    AssetManager& assetManager,
    EditorState& editorState,
    const std::filesystem::path& assetRoot
) {
    SceneRequestResult result{
        SceneRequestStatus::Rejected,
        "Unknown scene file command"
    };

    switch (command.type) {
        case SceneFileCommandType::None:
            return false;

        case SceneFileCommandType::Open:
            result = requestOpen(sceneManager, sceneSerializer, assetManager,
                                 editorState, assetRoot, command.path);
            break;

        case SceneFileCommandType::Save: {
            SceneFileResult saveResult = saveScene(
                sceneManager, sceneSerializer, assetManager, editorState, assetRoot
            );
            setSceneFileFeedback(saveResult.message);
            return false;
        }

        case SceneFileCommandType::SaveAs: {
            SceneFileResult saveResult = saveSceneAs(
                sceneManager, sceneSerializer, assetManager, editorState,
                assetRoot, command.path
            );
            setSceneFileFeedback(saveResult.message);
            return false;
        }

        case SceneFileCommandType::ConfirmSave:
            result = resolveUnsavedChanges(
                sceneManager, sceneSerializer, assetManager, editorState,
                assetRoot, UnsavedChangesChoice::Save
            );
            break;

        case SceneFileCommandType::ConfirmDiscard:
            result = resolveUnsavedChanges(
                sceneManager, sceneSerializer, assetManager, editorState,
                assetRoot, UnsavedChangesChoice::Discard
            );
            break;

        case SceneFileCommandType::ConfirmCancel:
            result = resolveUnsavedChanges(
                sceneManager, sceneSerializer, assetManager, editorState,
                assetRoot, UnsavedChangesChoice::Cancel
            );
            break;

        case SceneFileCommandType::ProvideSavePath:
            result = provideSavePath(
                sceneManager, sceneSerializer, assetManager, editorState,
                assetRoot, command.path
            );
            break;
    }

    setSceneFileFeedback(result.message);
    return result.status == SceneRequestStatus::ReadyToClose;
}
#endif

void engine() {
    GameApp gameApp;
    engine(gameApp);
}

void engine(GameApp& gameApp) {
    bool running = initializePlatform();

    if (!running) {
        logMessage(logger_level::Error,
                   "Error occurred while initializing engine");
        return;
    }

    try {
        initializeInput();

        {
            // Keep all engine systems alive until the main loop finishes.
            const std::filesystem::path assetRoot = getAssetRootPath();
            AudioService audioService(assetRoot);
            EventDispatcher eventDispatcher;
            ColliderSystem colliderSystem;
            SceneManager sceneManager(eventDispatcher);
            SceneSerializer sceneSerializer;

#ifdef ENGINE_EDITOR
            EditorState editorState;
            EngineStats engineStats;
            bool hasPreviousFrame = false;
#endif


            // Load the initial scene exactly once

            ProjectConfig projectConfig = loadProjectConfig(assetRoot);

            std::unique_ptr<Scene> loadedScene = sceneSerializer.load(resolveScenePath(assetRoot, projectConfig.startupScene.generic_string()), getPlatformAssetManager());

            sceneManager.setActiveScene(std::move(loadedScene));

#ifndef ENGINE_EDITOR
            const std::filesystem::path parentPath = projectConfig.startupScene.parent_path();
            const std::string stem = projectConfig.startupScene.stem().string();
            const std::string extension = projectConfig.startupScene.extension().string();

            const std::filesystem::path relativeSavedScenePath = parentPath / (stem + "-saved" + extension);

            const std::filesystem::path savedScenePath = resolveScenePath(assetRoot, relativeSavedScenePath.generic_string());
#endif


#ifdef ENGINE_EDITOR
            editorState.onSceneReplaced();
            editorState.setCurrentScenePath(projectConfig.startupScene);
            editorState.clearDirty();
#endif
            gameApp.onStart(audioService);
            
            sceneManager.update(0.0f);

            gameApp.onSceneChanged(sceneManager.getActiveScene(), projectConfig.startupScene);

            auto prevTime = std::chrono::steady_clock::now();

            while (running) {
                const auto currentTime = std::chrono::steady_clock::now();

                // 1. Input and window-close requests.
                const bool keepRunning = processPlatformEvents();
                if (!keepRunning) {
#ifdef ENGINE_EDITOR
                    if (editorState.getPendingAction() == PendingSceneAction::None &&
                        !isStandaloneSaveRequested()) {
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

                // 2. Time the frame, including while simulation is paused.
                const double elapsedSeconds =
                    std::chrono::duration<double>(currentTime - prevTime).count();
                const float deltaTime = static_cast<float>(elapsedSeconds);

#ifdef ENGINE_EDITOR
                if (hasPreviousFrame) {
                    engineStats.recordFrame(elapsedSeconds);
                }
                hasPreviousFrame = true;
                const bool simulate = !editorState.getPause();
#else
                const bool simulate = true;
#endif

                // 3. Run game simulation only when unpaused.
                if (simulate) {
                    gameApp.onUpdate(sceneManager.getActiveScene(), deltaTime);
                    sceneManager.update(deltaTime);
                }

#ifdef ENGINE_EDITOR
                // 4. Build editor panels before the collision snapshot so
                // Inspector changes appear in the current frame.
                const SceneFileCommand sceneFileCommand =
                    drawEditorPanels(sceneManager.getActiveScene(), editorState);
#endif

                // 5. Detect overlaps and update the editor's collision snapshot.
                const auto collisionPairs = colliderSystem.detectCollisions(
                    sceneManager.getActiveScene()
                );

#ifdef ENGINE_EDITOR
                editorState.setCollisions(collisionPairs);
#endif

                // 6. Notify the game of detected collisions
                gameApp.onCollisions(collisionPairs, sceneManager.getActiveScene(), audioService, simulate);

                // 6b. Remove entities that gameplay asked to remove this frame.
                // Safe here: updates and collision checks are finished.
                sceneManager.getActiveScene().applyPendingRemovals();

#ifdef ENGINE_EDITOR
                // 7. Render the scene ONCE, with overlay, stats and ImGui.
                StatsSnapshot stats;
                stats.fps = engineStats.getFPS();
                stats.averageFrameTimeMs = engineStats.getAverageFrameTimeMs();
                stats.entityCount =
                    sceneManager.getActiveScene().getEntities().size();
                stats.validColliderCount = colliderSystem.getValidColliderCount(
                    sceneManager.getActiveScene()
                );
                stats.overlapPairCount = collisionPairs.size();
                stats.cachedTextureCount =
                    getPlatformAssetManager().getCachedTextureCount();

                // renderPlatform fills spriteDrawCount after drawing the scene.
                renderPlatform(sceneManager.getActiveScene(), editorState, stats);

                // 8. Remove entities at the safe frame boundary, before a Save.
                if (auto deleteId = editorState.takePendingDelete()) {
                    if (sceneManager.getActiveScene().removeEntity(*deleteId)) {
                        if (editorState.getSelectedId() == deleteId) {
                            editorState.unselect();
                        }
                        editorState.markDirty();
                    }
                }

                // 9. Open/Save/Close only after the current frame has rendered.
                const std::uint64_t previousGeneration =
                    editorState.getSceneGeneration();

                if (processSceneFileCommand(
                        sceneFileCommand, sceneManager, sceneSerializer,
                        getPlatformAssetManager(), editorState, assetRoot)) {
                    break;
                }

                if (previousGeneration != editorState.getSceneGeneration()) {
                    auto scenePath = editorState.getCurrentScenePath();
                    if (scenePath.has_value()){
                        gameApp.onSceneChanged(sceneManager.getActiveScene(), scenePath.value());
                    }
                    editorState.setCollisions({});
                }
#else
                // 7. Non-editor builds render the scene normally.
                renderEngineFrame(sceneManager);    


                // 8. Legacy F5/F9 scene shortcuts for non-editor builds.
                if (wasKeyPressed(Key::F5)) {
                    try {
                        sceneSerializer.save(
                            sceneManager.getActiveScene(), savedScenePath,
                            getPlatformAssetManager()
                        );
                        logMessage(logger_level::Info,
                                   "Scene saved to: " + savedScenePath.string());
                    } catch (const std::exception& e) {
                        logMessage(logger_level::Error,
                                   std::string("Failed to save Scene: ") + e.what());
                    }
                }

                if (wasKeyPressed(Key::F9)) {
                    try {
                        if (!std::filesystem::exists(savedScenePath)) {
                            logMessage(logger_level::Warning,
                                       "No saved Scene found. Press F5 first.");
                        } else {
                            auto newScene = sceneSerializer.load(
                                savedScenePath, getPlatformAssetManager()
                            );
                            sceneManager.setActiveScene(std::move(newScene));
                            sceneManager.update(0.0f);
                             gameApp.onSceneChanged(sceneManager.getActiveScene(), relativeSavedScenePath);
                            logMessage(logger_level::Info,
                                       "Scene reloaded successfully!");
                        }
                    } catch (const std::exception& e) {
                        logMessage(logger_level::Error,
                                   std::string("Failed to reload Scene: ") +
                                       e.what());
                    }
                }
#endif

#ifdef ENGINE_EDITOR
                // 10. Don't toggle pause during a pending scene-file action.
                if (wasShortcutPressed(Key::F1) &&
                    editorState.getPendingAction() == PendingSceneAction::None &&
                    !isStandaloneSaveRequested()) {
                    if (editorState.getPause()) {
                        editorState.resume();
                    } else {
                        editorState.pause();
                    }
                }
#endif

                prevTime = currentTime;
            }
        }
        // Engine systems are destroyed before SDL shuts down.
    } catch (const std::exception& e) {
        logMessage(logger_level::Error,
                   std::string("Engine error: ") + e.what());
    } catch (...) {
        logMessage(logger_level::Error,
                   "An unknown error occurred in the engine");
    }

    shutdownPlatform();
}
