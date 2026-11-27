#include "behavior_component.h"
#include <stdexcept>
#include <utility>

BehaviorComponent::~BehaviorComponent() {
    // for each entry:
    //   if it was started → call cleanup() on its behavior
    for(auto& entry : entries){
        if(entry.started){
            entry.behavior->cleanup();
        }
    }
}

void BehaviorComponent::addBehavior(const std::string& name, std::unique_ptr<Behavior> behavior) {
    // 1. behavior is null (!behavior) → throw
    if(behavior==nullptr){
        throw std::invalid_argument("Behavior is null");
    }
    // 2. name already used (getBehavior(name) != nullptr) → throw
    if(getBehavior(name) != nullptr){
        throw std::invalid_argument("Behavior '" + name + "' is already on this entity");
    }
    // 3. add a new Entry to the list:
    entries.push_back({name, std::move(behavior)});
    
}

Behavior* BehaviorComponent::getBehavior(const std::string& name) const {
    // for each entry:
    //   if entry.name == name → return entry.behavior.get()
    for(auto& entry:entries){
        if(entry.name == name){
            return entry.behavior.get();
        }
    }
    // not found → return nullptr
    return nullptr;
}

std::vector<std::string> BehaviorComponent::getBehaviorNames() const {
    // 1. make an empty std::vector<std::string>
    std::vector<std::string> names;
    // 2. for each entry → push_back its name
    for(auto& entry:entries){
        names.push_back(entry.name);
    }
    // 3. return the vector
    return names;
}

void BehaviorComponent::update(float dt) {
    if (dt <= 0.0f) {
    return;
    }
    // for each entry:
    for(auto& entry: entries){
    //   1. make sure the behavior knows its entity: setOwner(getOwner())
    entry.behavior->setOwner(getOwner());
    //   2. not started yet → call start(), set started = true
    if(!entry.started){
        entry.behavior->start();
        entry.started=true;
    }
    //   3. call update(dt)
    entry.behavior->update(dt);
    }
}

void BehaviorComponent::onCollision(const CollisionEvent& event) {
    for (const auto &entry : entries){
        if (entry.started){
            entry.behavior->onCollision(event);
        }
    }
}