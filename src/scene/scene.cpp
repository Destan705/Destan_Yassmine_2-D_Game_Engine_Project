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

 // Asks to remove an entity at the end of the frame (safe during updates).
    void Scene::requestRemove(std::uint64_t id){
        pendingRemovals.push_back(id);
    }
    // Removes every requested entity. Call only when nothing is looping over entities.
    void Scene::applyPendingRemovals(){
        for(auto& removal: pendingRemovals){
            removeEntity(removal);
        }
        pendingRemovals.clear();

        std::erase_if(entities, [](const std::unique_ptr<Entity>& entity) {
            return entity->isRemovalRequested();
        });
    }