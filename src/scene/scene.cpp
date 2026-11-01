#include "scene.h"

Scene::Scene(std::string name) {
    sceneName = name;
}

std::string Scene::getSceneName() {
    return sceneName;
}

Entity& Scene::createEntity() {
    std::unique_ptr<Entity> entity = std::make_unique<Entity>(iDcounter);
    iDcounter++;
    entities.push_back(std::move(entity));
    return *entities.back();
}

void Scene::update(float dt) {
    for (auto& entity : entities) {
        entity->update(dt);
    }
}

void Scene::render(Renderer& renderer) {
    for (auto& entity : entities) {
        entity->render(renderer);
    }
}
