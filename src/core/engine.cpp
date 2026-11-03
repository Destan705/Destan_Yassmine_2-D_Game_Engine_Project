#include "animationComponent.h"
#include "entity.h"
#include "game.h"
#include "input.h"
#include "logger.h"
#include "platform.h"
#include "render_api.h"
#include "scene_manager.h"
#include "sprite.h"
#include "transform.h"
#include <SDL3/SDL.h>
#include <array>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

void renderEngineFrame(SceneManager& sceneManager) {
    renderPlatform(sceneManager.getActiveScene());
}

void engine() {

    bool running = initializePlatform();

    if (running) {
        initializeInput();
    }

    if (!running) {
        logMessage(logger_level::Error, "Error occured while initializing engine");
        return;
    }

    float delta_time = 0.0f;
    auto prev_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count();

    {
        EventDispatcher eventDispatcher;
        SceneManager sceneManager(
            eventDispatcher); // In C++, new SceneManager() returns a SceneManager* (pointer), but
                              // sceneManager is declared as a SceneManager object, not a pointer
        sceneManager.setActiveScene(std::make_unique<Scene>("Current Scene"));

        Entity& sceneEntity = sceneManager.getActiveScene().createEntity();
        Transform& transformComponent = sceneEntity.addComponent<Transform>();
        textureHandle handle = loadPlatformTexture("assets/test.bmp");
        sceneEntity.addComponent<Sprite>(handle);

        Entity& sceneEntity2 = sceneManager.getActiveScene().createEntity();
        Transform& transformComponent2 = sceneEntity2.addComponent<Transform>();
        textureHandle handle2 = loadPlatformTexture("assets/test.bmp");
        sceneEntity2.addComponent<Sprite>(handle2);

        Entity& sceneEntity3 = sceneManager.getActiveScene().createEntity();
        Transform& transformComponent3 = sceneEntity3.addComponent<Transform>();
        textureHandle handle3 = loadPlatformTexture("assets/anim_sheet.bmp");
        sceneEntity3.addComponent<Sprite>(handle3);

        transformComponent.x = 100.0f;
        transformComponent.y = 150.0f;
        transformComponent.width = 50.0f;
        transformComponent.height = 50.0f;

        transformComponent2.x = 200.0f;
        transformComponent2.y = 300.0f;
        transformComponent2.width = 50.0f;
        transformComponent2.height = 50.0f;

        transformComponent3.x = 300.0f;
        transformComponent3.y = 150.0f;
        transformComponent3.width = 32.0f;
        transformComponent3.height = 32.0f;
        std::vector<Bounds> frames = {
            {0.0f, 0.0f, 32.0f, 32.0f},
            {32.0f, 0.0f, 32.0f, 32.0f},
            {64.0f, 0.0f, 32.0f, 32.0f},
            {96.0f, 0.0f, 32.0f, 32.0f},
        };
        sceneEntity3.addComponent<AnimationComponent>(frames, 0.25f, true);

        while (running) {
            auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::steady_clock::now().time_since_epoch())
                                    .count();

            //  Process Input
            running = processPlatformEvents();

            if (!running) {
                break;
            }

            //  Update Engine State
            delta_time = (current_time - prev_time) / 1000.0f;

            sceneManager.update(delta_time);

            //  Render Engine Frame
            renderEngineFrame(sceneManager);

            prev_time = current_time;
        }
    }
    // Shutdown system
    shutdownPlatform();
}