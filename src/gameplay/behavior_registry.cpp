#include "behavior_registry.h"
#include <stdexcept>
#include <utility>

bool BehaviorRegistry::has(const std::string& name) const {
    return factories.count(name) > 0;
}

void BehaviorRegistry::registerBehavior(const std::string& name, BehaviorFactory factory) {
    // 1. name empty → throw
    if(name==""){
        throw std::invalid_argument("Name empty string");
    }
    // 2. factory empty (!factory) → throw
    if(!factory){
        throw std::invalid_argument("Factory is empty");
    }
    // 3. has(name) → throw
    if(has(name)){
        throw std::invalid_argument("Behavior '" + name + "' is already registered");
    }
    // 4. store it
    factories[name] = std::move(factory);
}

std::unique_ptr<Behavior> BehaviorRegistry::create(const std::string& name) const {
    // 1. look up the name
    auto it = factories.find(name);
    // 2. not found (it == factories.end()) → throw
    if(it==factories.end()){
        throw std::out_of_range("Unknown behavior '" + name + "'");
    }
    // 3. call the factory and return the result
    return it->second();
}