#pragma once
#include "entity.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Scene {
  public:
    explicit Scene(std::string name);
    std::string getSceneName(); // gives the name back when askes
    Entity& createEntity(); // makes new entity stores, gives it the next ID, stores it, and hands
                            // it back so you can add components.
    void update(float dt);  // tells every entity to update, in order they were made
    void render(Renderer&); // tells every entity to draw, in the same order

  private:
    std::string sceneName;                         // names of the scenes
    std::vector<std::unique_ptr<Entity>> entities; // each scene's entity
    std::uint64_t iDcounter = 1;                   // idCounter for entity in scenes
};
