#include "scene.h"

Scene::Scene(std::string name) {
    sceneName = name;
}

std::string Scene::getSceneName() const{
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

const std::vector<std::unique_ptr<Entity>>& Scene::getEntities() const{
  return entities; 
 }
