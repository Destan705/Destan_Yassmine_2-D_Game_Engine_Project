#pragma once
#include "component.h"
#include "behavior.h"
#include <memory>
#include <string>
#include <vector>

// Holds several named gameplay behaviors on one entity
// (for example "KeyboardMove" and "Coin").
// The entity still has only one BehaviorComponent, so the
// one-component-per-type rule is kept.
class BehaviorComponent : public Component {
public:
    // Calls cleanup() on every behavior that was started.
    ~BehaviorComponent() override;

    // Adds a behavior under a name. Throws std::invalid_argument
    // if the behavior is null or the name is already used on this entity.
    void addBehavior(const std::string& name, std::unique_ptr<Behavior> behavior);

    // The behavior with this name, or nullptr if there is none.
    Behavior* getBehavior(const std::string& name) const;

    // Names in the order they were added (used later for saving).
    std::vector<std::string> getBehaviorNames() const;
    // Called once per simulation frame (never while paused).
    // Order: entities in creation order, then behaviors in the order they were added.
    void update(float dt) override;

    void onCollision(const CollisionEvent& event);

private:
    struct Entry {
        std::string name;
        std::unique_ptr<Behavior> behavior;
        bool started = false;
    };

    std::vector<Entry> entries;
};