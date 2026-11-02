#include "colliderSystem.h"
#include "scene.h"
#include <optional>
#include "entity.h"
#include "transform.h"
#include "colliderComponent.h"
#include <cmath>



namespace{
    struct rectangle // AABB struct to simplify checks (2 rectangles vs 8 float values)
    {
        float left;
        float right;
        float top;
        float bottom;
    };

    bool overlaps(const rectangle& rectangleA, const rectangle& rectangleB) {
        bool horizontalCheck = (rectangleA.right > rectangleB.left) && (rectangleA.left < rectangleB.right);
        bool verticalCheck = (rectangleA.bottom > rectangleB.top) && (rectangleA.top < rectangleB.bottom);
        return (horizontalCheck && verticalCheck);
    }

    std::optional<rectangle> getWorldBounds(Entity& entity) {
        Transform* transformComponent = entity.getComponent<Transform>();
        ColliderComponent* colliderComponent = entity.getComponent<ColliderComponent>();

        if (!transformComponent || !colliderComponent || !colliderComponent->isEnabled()) {
            return std::nullopt;
        }
        
        rectangle bounds;
        bounds.left = transformComponent->x + colliderComponent->getOffsetX();
        bounds.top = transformComponent->y + colliderComponent->getOffsetY();
        bounds.right = bounds.left + colliderComponent->getWidth();
        bounds.bottom = bounds.top + colliderComponent->getHeight();

        if (!std::isfinite(bounds.left) || !std::isfinite(bounds.top) || !std::isfinite(bounds.right) || !std::isfinite(bounds.bottom) ) {
            return std::nullopt;
        }

        if (bounds.right > bounds.left || bounds.bottom > bounds.top) {
            return std::nullopt;
        }

        return bounds;
    }

}


std::vector<ColliderSystem::CollisionPair> ColliderSystem::detectCollisions(Scene& scene) const {

    std::vector<CollisionPair> collidingEntities;

    // Access the Scene's entities without copying them
    const auto& sceneEntities = scene.getEntities();

    // Go through each unique pair of entities
    for (std::size_t i = 0; i < sceneEntities.size(); i++) {
        Entity& entityA = *sceneEntities[i];

        // Get A's collision rectangle
        auto rectangleEntityA = getWorldBounds(entityA);

        // Skip if A has no valid collider
        if (!rectangleEntityA) {
            continue;
        }

        for (std::size_t j = i + 1; j < sceneEntities.size(); j++) {
            Entity& entityB = *sceneEntities[j];

            // Get B's collision rectangle
            auto rectangleEntityB = getWorldBounds(entityB);

            // Skip if B has no valid collider
            if (!rectangleEntityB) {
                continue;
            }

            // Check whether the two rectangles overlap
            if (overlaps(*rectangleEntityA, *rectangleEntityB)) {
                collidingEntities.emplace_back(entityA.getId(), entityB.getId());
            }
        }
    }

    return collidingEntities;
}

