#include "scene_manager.h"
#include "logger.h"
#include <stdexcept>
#include <string>
#include <utility>
#include <exception>

SceneManager::SceneManager(EventDispatcher& dispatcher) : eventDispatcher(dispatcher) {
}

// switches to a new scene
void SceneManager::setActiveScene(std::unique_ptr<Scene> scene) {
    if (scene == nullptr) {
        throw std::invalid_argument("Scene cannot be null");
    }

    std::string oldName;
    if (!(currentScene == nullptr)) {
        oldName = currentScene->getSceneName();
    }

    std::string newName = scene->getSceneName();

    currentScene = std::move(scene);

    try {
        eventDispatcher.dispatch(SceneChangedEvent{oldName, newName});
    } catch (const std::exception& e) {
        logMessage(logger_level::Error, std::string("Scene-change notification failed: ") + e.what());
    } catch (...) {
        logMessage(logger_level::Error, "Unknown exception occurred during scene-change");
    }
}

// upadates the active scene
void SceneManager::update(float dt) {
    if (!(currentScene == nullptr)) {
        currentScene->update(dt);
    }
}
// gives back the current scene
Scene& SceneManager::getActiveScene() {
    if (currentScene == nullptr) {
        throw std::runtime_error("no active scene");
    }
    return *currentScene;
}
