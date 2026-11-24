#pragma once
#include "behavior.h"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

// A function that makes a new behavior.
using BehaviorFactory = std::function<std::unique_ptr<Behavior>()>;

// Maps behavior names (as written in scene files) to factories that create them.
// Game code registers its behaviors here; the engine creates them by name.
class BehaviorRegistry {
public:
    // Saves a factory under a name.
    // Throws std::invalid_argument if the name is empty, already taken, or the factory is empty.
    void registerBehavior(const std::string& name, BehaviorFactory factory);

    // Builds a new behavior from its name.
    // Throws std::out_of_range if the name was never registered.
    std::unique_ptr<Behavior> create(const std::string& name) const;

    // True if a behavior with this name is registered.
    bool has(const std::string& name) const;

private:
    std::unordered_map<std::string, BehaviorFactory> factories;
};