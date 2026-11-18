#include "scene.h"

Scene::Scene(std::string name) {
    sceneName = name;
}

std::string Scene::getSceneName() const {
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

const std::vector<std::unique_ptr<Entity>>& Scene::getEntities() const {
    return entities;
}

// Removes the entity with this ID. Returns false if no entity has it.
// Never call this while looping over getEntities() (it changes the list).
bool Scene::removeEntity(std::uint64_t id) {
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        if ((*it)->getId() == id) {
            entities.erase(it);   // the unique_ptr frees the Entity and its components
            return true;
        }
    }
    return false;
}
