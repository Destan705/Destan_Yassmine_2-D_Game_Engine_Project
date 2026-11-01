#include "platform.h"
#include "transform.h"
#include "sprite.h"
#include "render_api.h"
#include "scene_manager.h"
#include "entity.h"
#include "logger.h"
#include "input.h"
#include "game.h"
#include <iostream>
#include <chrono>
#include <array>
#include <memory>
#include <SDL3/SDL.h>
#include <string>

using namespace std;



void renderEngineFrame(SceneManager& sceneManager) {
    renderPlatform(sceneManager.getActiveScene());
}


void engine() {

    bool running = initializePlatform();

    if (running) { initializeInput();}

    if (!running) {
        logMessage(logger_level::Error, "Error occured while initializing engine");
        return;
    }


    float delta_time = 0.0f;
    auto prev_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    {
        EventDispatcher eventDispatcher;
        SceneManager sceneManager(eventDispatcher); //In C++, new SceneManager() returns a SceneManager* (pointer), but sceneManager is declared as a SceneManager object, not a pointer
        sceneManager.setActiveScene(std::make_unique<Scene>("Current Scene"));

        Entity& sceneEntity = sceneManager.getActiveScene().createEntity();
        Transform& transformComponent = sceneEntity.addComponent<Transform>();
        textureHandle handle = loadPlatformTexture("assets/test.bmp");
        sceneEntity.addComponent<Sprite>(handle);

        transformComponent.x = 100.0f;
        transformComponent.y = 150.0f;
        transformComponent.width = 50.0f;
        transformComponent.height = 50.0f;

        while (running) {
            auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

            //  Process Input
            running = processPlatformEvents();
            
            if (!running) {break;}

            //  Update Engine State
            delta_time = (current_time - prev_time)/1000.0f;

            sceneManager.update(delta_time);


            //  Render Engine Frame
            renderEngineFrame(sceneManager);

            prev_time = current_time;
        }

    }
    // Shutdown system
    shutdownPlatform();

}