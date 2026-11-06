#pragma once

#include "component.h"
#include <cstdint>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <vector>

class Entity {

  public:
    explicit Entity(std::uint64_t id);

    ~Entity() = default;

    Entity(const Entity&) = delete; // Prevents creating a new Entity by copying an existing one

    Entity& operator=(const Entity&) =
        delete; // Prevents assigning an existing Entity’s contents to another

    Entity(Entity&&) = delete; // Prevents creating a new Entity by moving an existing one

    Entity&
    operator=(Entity&&) = delete; // Prevents moving an existing Entity’s contents into another

    std::uint64_t getId() const;

    void update(float deltaTime);

    void render(Renderer& renderer);

    template <typename T>
    // For each component, check whether its actual type is T
    // If true, return a pointer to it
    // If the entire search finishes without a match, return nullptr
    T* getComponent() {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        for (const auto& component : components) {
            if (typeid(*component) == typeid(T)) {
                return static_cast<T*>(component.get());
            }
        }
        return nullptr;
    }

    template <typename T, typename... Args>
    // Creates a component of type T using the supplied constructor arguments
    // Throws std::logic_error if this Entity already has a component of that type
    // Sets its owner to this Entity and transfers ownership into the components vector
    // Returns a reference for access; the Entity owns and automatically destroys it
    T& addComponent(Args&&... args) {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
        if (getComponent<T>() != nullptr) {
            throw std::logic_error("This component already exists");
        }
        std::unique_ptr<T> newComponent = std::make_unique<T>(std::forward<Args>(args)...);
        newComponent->setOwner(*this);
        T& attachedComponent = *newComponent;
        components.push_back(std::move(newComponent));
        return attachedComponent;
    }

  private:
    std::uint64_t id;

    std::vector<std::unique_ptr<Component>> components;
};