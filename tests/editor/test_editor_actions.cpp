#include "editor_actions.h"
#include "transform_component.h"
#include "collider_component.h"
#include <gtest/gtest.h>
#include <limits>

TEST(EditorActionsTest, RejectsEditingEntityWithoutTransform) {
    Entity entity(1);
    EXPECT_EQ(applyTransformEdit(entity, 0, 0, 20, 20), EditResult::Rejected);
}

TEST(EditorActionsTest, NegativePositionAndPositiveDimensionsAreValid) {
    Entity entity(1);
    auto& transform = entity.addComponent<TransformComponent>();
    EXPECT_EQ(applyTransformEdit(entity, -25, -10, 12, 14), EditResult::Applied);
    EXPECT_FLOAT_EQ(transform.x, -25);
    EXPECT_FLOAT_EQ(transform.y, -10);
    EXPECT_FLOAT_EQ(transform.width, 12);
    EXPECT_FLOAT_EQ(transform.height, 14);
    EXPECT_EQ(applyTransformEdit(entity, -25, -10, 12, 14), EditResult::NoChange);
}

TEST(EditorActionsTest, RejectsNaNInfinityAndNonpositiveDimensionsWithoutMutation) {
    Entity entity(1);
    auto& transform = entity.addComponent<TransformComponent>();
    ASSERT_EQ(applyTransformEdit(entity, 1, 2, 3, 4), EditResult::Applied);
    const auto nan = std::numeric_limits<float>::quiet_NaN();
    const auto inf = std::numeric_limits<float>::infinity();
    EXPECT_EQ(applyTransformEdit(entity, nan, 2, 3, 4), EditResult::Rejected);
    EXPECT_EQ(applyTransformEdit(entity, 1, inf, 3, 4), EditResult::Rejected);
    EXPECT_EQ(applyTransformEdit(entity, 1, 2, 0, 4), EditResult::Rejected);
    EXPECT_EQ(applyTransformEdit(entity, 1, 2, 3, -1), EditResult::Rejected);
    EXPECT_FLOAT_EQ(transform.x, 1);
    EXPECT_FLOAT_EQ(transform.y, 2);
    EXPECT_FLOAT_EQ(transform.width, 3);
    EXPECT_FLOAT_EQ(transform.height, 4);
}

TEST(EditorActionsTest, ColliderEditReturnsAppliedNoChangeAndRejected) {
    Entity entity(1);
    auto& collider = entity.addComponent<ColliderComponent>(0, 0, 10, 10);
    EXPECT_EQ(applyColliderEdit(entity, -2, 3, 20, 30, false), EditResult::Applied);
    EXPECT_FLOAT_EQ(collider.getOffsetX(), -2);
    EXPECT_FALSE(collider.isEnabled());
    EXPECT_EQ(applyColliderEdit(entity, -2, 3, 20, 30, false), EditResult::NoChange);
    EXPECT_EQ(applyColliderEdit(entity, -2, 3, 0, 30, true), EditResult::Rejected);
    EXPECT_FALSE(collider.isEnabled());
    EXPECT_FLOAT_EQ(collider.getWidth(), 20);
}

TEST(EditorActionsTest, CreateAndRenameMarkSceneDirtyOnlyForRealChanges) {
    Scene scene("Editor");
    EditorState state;
    const auto id = createEntityAction(scene, state);
    EXPECT_TRUE(state.isDirty());
    ASSERT_EQ(scene.getEntities().size(), 1u);
    auto& entity = *scene.getEntities().front();
    EXPECT_EQ(entity.getId(), id);
    EXPECT_NE(entity.getComponent<TransformComponent>(), nullptr);
    state.clearDirty();
    EXPECT_EQ(renameEntityAction(entity, entity.getName(), state), EditResult::NoChange);
    EXPECT_FALSE(state.isDirty());
    EXPECT_EQ(renameEntityAction(entity, "   ", state), EditResult::Rejected);
    EXPECT_FALSE(state.isDirty());
    EXPECT_EQ(renameEntityAction(entity, "  Player  ", state), EditResult::Applied);
    EXPECT_TRUE(state.isDirty());
    EXPECT_EQ(entity.getName(), "Player");
}
