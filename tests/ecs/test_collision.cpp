#include "colliderComponent.h"
#include "colliderSystem.h"
#include "entity.h"
#include "scene.h"
#include "transform.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

using Pair = ColliderSystem::CollisionPair;

// Adds an entity with a Transform at (x, y) and a collider covering the same area.
Entity& AddBox(Scene& scene, float x, float y, float width, float height, bool enabled = true) {
    Entity& entity = scene.createEntity();
    Transform& transform = entity.addComponent<Transform>();
    transform.x = x;
    transform.y = y;
    transform.width = width;
    transform.height = height;
    entity.addComponent<ColliderComponent>(0.0f, 0.0f, width, height, enabled);
    return entity;
}

std::vector<Pair> Detect(Scene& scene) {
    return ColliderSystem{}.detectCollisions(scene);
}

// ============================================================
// ColliderComponent
// ============================================================

TEST(ColliderComponentTest, StoresLocalBounds) {
    ColliderComponent collider(5.0f, -3.0f, 20.0f, 10.0f);
    EXPECT_FLOAT_EQ(collider.getOffsetX(), 5.0f);
    EXPECT_FLOAT_EQ(collider.getOffsetY(), -3.0f);
    EXPECT_FLOAT_EQ(collider.getWidth(), 20.0f);
    EXPECT_FLOAT_EQ(collider.getHeight(), 10.0f);
}

TEST(ColliderComponentTest, EnabledByDefault) {
    ColliderComponent collider(0.0f, 0.0f, 1.0f, 1.0f);
    EXPECT_TRUE(collider.isEnabled());
}

TEST(ColliderComponentTest, CanBeCreatedDisabledAndToggled) {
    ColliderComponent collider(0.0f, 0.0f, 1.0f, 1.0f, false);
    EXPECT_FALSE(collider.isEnabled());
    collider.setEnabled(true);
    EXPECT_TRUE(collider.isEnabled());
}

TEST(ColliderComponentTest, RejectsNonPositiveSize) {
    EXPECT_THROW(ColliderComponent(0.0f, 0.0f, 0.0f, 10.0f), std::invalid_argument);
    EXPECT_THROW(ColliderComponent(0.0f, 0.0f, 10.0f, -1.0f), std::invalid_argument);
}

TEST(ColliderComponentTest, RejectsNonFiniteValues) {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float inf = std::numeric_limits<float>::infinity();
    EXPECT_THROW(ColliderComponent(nan, 0.0f, 1.0f, 1.0f), std::invalid_argument);
    EXPECT_THROW(ColliderComponent(0.0f, inf, 1.0f, 1.0f), std::invalid_argument);
    EXPECT_THROW(ColliderComponent(0.0f, 0.0f, inf, 1.0f), std::invalid_argument);
    EXPECT_THROW(ColliderComponent(0.0f, 0.0f, 1.0f, nan), std::invalid_argument);
}

TEST(ColliderComponentTest, FailedUpdateKeepsPreviousBounds) {
    ColliderComponent collider(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_THROW(collider.setLocalBounds(9.0f, 9.0f, -5.0f, 9.0f), std::invalid_argument);
    EXPECT_FLOAT_EQ(collider.getOffsetX(), 1.0f);
    EXPECT_FLOAT_EQ(collider.getOffsetY(), 2.0f);
    EXPECT_FLOAT_EQ(collider.getWidth(), 3.0f);
    EXPECT_FLOAT_EQ(collider.getHeight(), 4.0f);
}

// ============================================================
// ColliderSystem: overlap rules
// ============================================================

TEST(ColliderSystemTest, OverlappingBoxesAreReported) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    AddBox(scene, 5, 5, 10, 10);
    EXPECT_EQ(Detect(scene), (std::vector<Pair>{{1, 2}}));
}

TEST(ColliderSystemTest, TouchingEdgesIsNotACollision) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    AddBox(scene, 10, 0, 10, 10); // right edge of A == left edge of B
    AddBox(scene, 0, 10, 10, 10); // bottom edge of A == top edge of C
    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, TouchingCornersIsNotACollision) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    AddBox(scene, 10, 10, 10, 10);
    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, TinyOverlapPastTheEdgeIsACollision) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    AddBox(scene, 9.5f, 0, 10, 10);
    EXPECT_EQ(Detect(scene).size(), 1u);
}

TEST(ColliderSystemTest, SeparatedOnOneAxisIsNotACollision) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    AddBox(scene, 5, 20, 10, 10); // overlaps horizontally, not vertically
    AddBox(scene, 20, 5, 10, 10); // overlaps vertically, not horizontally
    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, ContainmentIsACollision) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 100, 100);
    AddBox(scene, 40, 40, 10, 10);
    EXPECT_EQ(Detect(scene), (std::vector<Pair>{{1, 2}}));
}

TEST(ColliderSystemTest, IdenticalBoxesCollide) {
    Scene scene("Test");
    AddBox(scene, 3, 3, 5, 5);
    AddBox(scene, 3, 3, 5, 5);
    EXPECT_EQ(Detect(scene).size(), 1u);
}

TEST(ColliderSystemTest, WorksWithNegativePositions) {
    Scene scene("Test");
    AddBox(scene, -20, -20, 15, 15);
    AddBox(scene, -10, -10, 15, 15);
    EXPECT_EQ(Detect(scene).size(), 1u);
}

// ============================================================
// ColliderSystem: offsets and current Transform
// ============================================================

TEST(ColliderSystemTest, OffsetIsAddedToTransformPosition) {
    Scene scene("Test");
    AddBox(scene, 50, 0, 10, 10);

    // Transform at (0, 0), but the collider is shifted right by 45, so it reaches 45..55.
    Entity& shifted = scene.createEntity();
    shifted.addComponent<Transform>();
    shifted.addComponent<ColliderComponent>(45.0f, 0.0f, 10.0f, 10.0f);

    EXPECT_EQ(Detect(scene).size(), 1u);
}

TEST(ColliderSystemTest, OffsetCanMoveColliderAwayFromSprite) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);

    // Same Transform position, but the collider is pushed far below.
    Entity& entity = scene.createEntity();
    entity.addComponent<Transform>();
    entity.addComponent<ColliderComponent>(0.0f, 100.0f, 10.0f, 10.0f);

    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, UsesCurrentTransformEveryPass) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    Entity& mover = AddBox(scene, 100, 0, 10, 10);
    EXPECT_TRUE(Detect(scene).empty());

    mover.getComponent<Transform>()->x = 5;
    EXPECT_EQ(Detect(scene).size(), 1u);

    mover.getComponent<Transform>()->x = 100;
    EXPECT_TRUE(Detect(scene).empty());
}

// ============================================================
// ColliderSystem: what gets skipped
// ============================================================

TEST(ColliderSystemTest, DisabledCollidersAreSkipped) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    AddBox(scene, 0, 0, 10, 10, false);
    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, ReenabledColliderIsDetectedAgain) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    Entity& toggled = AddBox(scene, 0, 0, 10, 10, false);
    EXPECT_TRUE(Detect(scene).empty());

    toggled.getComponent<ColliderComponent>()->setEnabled(true);
    EXPECT_EQ(Detect(scene).size(), 1u);
}

TEST(ColliderSystemTest, EntityWithoutTransformIsSkipped) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    Entity& noTransform = scene.createEntity();
    noTransform.addComponent<ColliderComponent>(0.0f, 0.0f, 10.0f, 10.0f);
    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, EntityWithoutColliderIsSkipped) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    Transform& transform = scene.createEntity().addComponent<Transform>();
    transform.width = 10;
    transform.height = 10;
    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, NonFiniteTransformIsSkipped) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    Entity& broken = AddBox(scene, 0, 0, 10, 10);
    broken.getComponent<Transform>()->x = std::numeric_limits<float>::infinity();
    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, EmptySceneReportsNothing) {
    Scene scene("Empty");
    EXPECT_TRUE(Detect(scene).empty());
}

// ============================================================
// ColliderSystem: pair reporting
// ============================================================

TEST(ColliderSystemTest, SingleEntityNeverCollidesWithItself) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    EXPECT_TRUE(Detect(scene).empty());
}

TEST(ColliderSystemTest, EachPairIsReportedOnceInInsertionOrder) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10); // 1
    AddBox(scene, 1, 1, 10, 10); // 2
    AddBox(scene, 2, 2, 10, 10); // 3

    // All three overlap: each unordered pair once, lower ID first, in scene order.
    EXPECT_EQ(Detect(scene), (std::vector<Pair>{{1, 2}, {1, 3}, {2, 3}}));
}

TEST(ColliderSystemTest, OnlyOverlappingPairsAreReported) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);   // 1
    AddBox(scene, 100, 0, 10, 10); // 2, far away
    AddBox(scene, 5, 5, 10, 10);   // 3, overlaps 1
    EXPECT_EQ(Detect(scene), (std::vector<Pair>{{1, 3}}));
}

TEST(ColliderSystemTest, RepeatedDetectionGivesSameResult) {
    Scene scene("Test");
    AddBox(scene, 0, 0, 10, 10);
    AddBox(scene, 5, 5, 10, 10);
    const auto first = Detect(scene);
    EXPECT_EQ(Detect(scene), first); // no state kept between passes
}

TEST(ColliderSystemTest, NewSceneOnlyReportsItsOwnEntities) {
    ColliderSystem system;

    Scene oldScene("Old");
    AddBox(oldScene, 0, 0, 10, 10);
    AddBox(oldScene, 5, 5, 10, 10);
    AddBox(oldScene, 6, 6, 10, 10);
    EXPECT_EQ(system.detectCollisions(oldScene).size(), 3u);

    Scene newScene("New");
    AddBox(newScene, 0, 0, 10, 10);
    AddBox(newScene, 50, 50, 10, 10);
    EXPECT_TRUE(system.detectCollisions(newScene).empty());

    AddBox(newScene, 52, 52, 10, 10); // ID 3 in the new scene
    EXPECT_EQ(system.detectCollisions(newScene), (std::vector<Pair>{{2, 3}}));
}

} // namespace
