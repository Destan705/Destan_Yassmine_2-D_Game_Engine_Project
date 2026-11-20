#include "entity.h"
#include "transform_component.h"
#include "sprite_component.h"
#include "collider_component.h"
#include "animation_component.h"
#include "scene.h"
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

// Covers the component-management data contract exercised by the Inspector.
// Widget focus/buffer rendering requires the manual ImGui checklist.
TEST(InspectorComponentTest, SupportsAddingAndRemovingColliderWithoutAffectingTransform) {
    Scene scene("Editor");
    auto& entity = scene.createEntity();
    entity.addComponent<TransformComponent>();
    entity.addComponent<ColliderComponent>(0, 0, 32, 32, true);
    EXPECT_NE(entity.getComponent<ColliderComponent>(), nullptr);
    EXPECT_TRUE(entity.removeComponent<ColliderComponent>());
    EXPECT_EQ(entity.getComponent<ColliderComponent>(), nullptr);
    EXPECT_NE(entity.getComponent<TransformComponent>(), nullptr);
    EXPECT_FALSE(entity.removeComponent<ColliderComponent>());
}

TEST(InspectorComponentTest, SupportsAddingAndRemovingSpriteWithoutAffectingTransform) {
    Entity entity(1);
    entity.addComponent<TransformComponent>();
    entity.addComponent<SpriteComponent>(1u);
    EXPECT_NE(entity.getComponent<SpriteComponent>(), nullptr);
    EXPECT_TRUE(entity.removeComponent<SpriteComponent>());
    EXPECT_EQ(entity.getComponent<SpriteComponent>(), nullptr);
    EXPECT_NE(entity.getComponent<TransformComponent>(), nullptr);
}

TEST(InspectorComponentTest, PreventsDuplicateComponentCreation) {
    Entity entity(1);
    entity.addComponent<ColliderComponent>(0, 0, 5, 5);
    EXPECT_THROW(entity.addComponent<ColliderComponent>(0, 0, 5, 5), std::logic_error);
}

TEST(InspectorComponentTest, AnimationCanBeRemovedWhileSpriteRemains) {
    Entity entity(1);
    entity.addComponent<SpriteComponent>(1u);
    entity.addComponent<AnimationComponent>(std::vector<Bounds>{{0, 0, 8, 8}}, 0.1f, true);
    EXPECT_TRUE(entity.removeComponent<AnimationComponent>());
    EXPECT_NE(entity.getComponent<SpriteComponent>(), nullptr);
}
