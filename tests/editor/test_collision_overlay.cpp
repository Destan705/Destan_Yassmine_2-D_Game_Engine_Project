#include "collider_overlay.h"
#include "collider_component.h"
#include "collider_system.h"
#include "scene.h"
#include "transform_component.h"
#include <gtest/gtest.h>
#include <limits>

namespace {
Entity& AddCollider(Scene& scene, float x = 0, float y = 0,
                    float offsetX = 0, float offsetY = 0,
                    float width = 10, float height = 10, bool enabled = true) {
    auto& entity = scene.createEntity();
    auto& transform = entity.addComponent<TransformComponent>();
    transform.x = x;
    transform.y = y;
    entity.addComponent<ColliderComponent>(offsetX, offsetY, width, height, enabled);
    return entity;
}
}

TEST(ColliderOverlayTest, AppliesTransformAndLocalOffsets) {
    Scene scene("Offsets");
    AddCollider(scene, 20, -5, 3, 7, 9, 11);
    EditorState state;
    auto outlines = collectColliderOutlines(scene, state);
    ASSERT_EQ(outlines.size(), 1u);
    EXPECT_FLOAT_EQ(outlines[0].bounds.x, 23.0f);
    EXPECT_FLOAT_EQ(outlines[0].bounds.y, 2.0f);
    EXPECT_FLOAT_EQ(outlines[0].bounds.w, 9.0f);
    EXPECT_FLOAT_EQ(outlines[0].bounds.h, 11.0f);
    EXPECT_EQ(outlines[0].color.green, 255);
}

TEST(ColliderOverlayTest, HidingOverlayReturnsNoOutlinesAndDoesNotModifyColliders) {
    Scene scene("Hidden");
    auto& entity = AddCollider(scene);
    EditorState state;
    state.setShowColliders(false);
    EXPECT_TRUE(collectColliderOutlines(scene, state).empty());
    EXPECT_TRUE(entity.getComponent<ColliderComponent>()->isEnabled());
    state.setShowColliders(true);
    EXPECT_EQ(collectColliderOutlines(scene, state).size(), 1u);
}

TEST(ColliderOverlayTest, SkipsMissingTransformOrColliderAndDisabledCollider) {
    Scene scene("Skip");
    scene.createEntity().addComponent<ColliderComponent>(0, 0, 10, 10);
    scene.createEntity().addComponent<TransformComponent>();
    AddCollider(scene, 0, 0, 0, 0, 10, 10, false);
    EXPECT_TRUE(collectColliderOutlines(scene, EditorState{}).empty());
}

TEST(ColliderOverlayTest, SkipsNonfiniteBoundsAndOverflow) {
    Scene scene("Broken");
    auto& entity = AddCollider(scene);
    entity.getComponent<TransformComponent>()->x = std::numeric_limits<float>::infinity();
    EXPECT_TRUE(collectColliderOutlines(scene, EditorState{}).empty());
    entity.getComponent<TransformComponent>()->x = std::numeric_limits<float>::max();
    entity.getComponent<ColliderComponent>()->setLocalBounds(0, 0, std::numeric_limits<float>::max(), 10);
    EXPECT_TRUE(collectColliderOutlines(scene, EditorState{}).empty());
}

TEST(ColliderOverlayTest, OverlapsUseRedAndSelectionOverridesWithYellow) {
    Scene scene("Overlaps");
    auto& a = AddCollider(scene, 0, 0);
    auto& b = AddCollider(scene, 5, 5);
    EditorState state;
    state.setCollisions(ColliderSystem{}.detectCollisions(scene));
    auto outlines = collectColliderOutlines(scene, state);
    ASSERT_EQ(outlines.size(), 2u);
    EXPECT_EQ(outlines[0].color.red, 255);
    EXPECT_EQ(outlines[0].color.green, 0);
    EXPECT_EQ(outlines[1].color.red, 255);
    state.select(a.getId());
    outlines = collectColliderOutlines(scene, state);
    EXPECT_EQ(outlines[0].color.red, 255);
    EXPECT_EQ(outlines[0].color.green, 255);
    EXPECT_EQ(outlines[0].color.blue, 0);
    EXPECT_EQ(outlines[1].entityId, b.getId());
    EXPECT_EQ(outlines[1].color.green, 0);
}

TEST(ColliderOverlayTest, EdgeAndCornerTouchingStayGreen) {
    Scene scene("Contacts");
    AddCollider(scene, 0, 0);
    AddCollider(scene, 10, 0);
    AddCollider(scene, 10, 10);
    EditorState state;
    state.setCollisions(ColliderSystem{}.detectCollisions(scene));
    EXPECT_TRUE(state.getCollisions().empty());
    for (const auto& outline : collectColliderOutlines(scene, state)) {
        EXPECT_EQ(outline.color.green, 255);
        EXPECT_EQ(outline.color.red, 0);
    }
}

TEST(ColliderOverlayTest, MovingWhilePausedCanRefreshSnapshotBeforeRendering) {
    Scene scene("Move");
    AddCollider(scene, 0, 0);
    auto& mover = AddCollider(scene, 50, 0);
    EditorState state;
    ColliderSystem system;
    state.setCollisions(system.detectCollisions(scene));
    EXPECT_TRUE(state.getCollisions().empty());
    mover.getComponent<TransformComponent>()->x = 5;
    state.setCollisions(system.detectCollisions(scene));
    auto outlines = collectColliderOutlines(scene, state);
    EXPECT_EQ(state.getCollisions().size(), 1u);
    EXPECT_EQ(outlines[0].color.green, 0);
}

TEST(ColliderOverlayTest, SceneReplacementDoesNotCarryStaleCollisionIds) {
    Scene oldScene("Old");
    AddCollider(oldScene);
    AddCollider(oldScene, 1, 1);
    EditorState state;
    state.setCollisions(ColliderSystem{}.detectCollisions(oldScene));
    Scene newScene("New");
    AddCollider(newScene);
    state.onSceneReplaced();
    auto outlines = collectColliderOutlines(newScene, state);
    ASSERT_EQ(outlines.size(), 1u);
    EXPECT_EQ(outlines[0].color.green, 255);
    EXPECT_EQ(outlines[0].color.red, 0);
}
