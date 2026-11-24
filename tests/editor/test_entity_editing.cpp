// Additional Phase 4 Hierarchy regression tests.
// These test pure scene/editor actions. ImGui confirmation requires a manual UI check.
#include "editor_actions.h"
#include "editor_state.h"
#include "scene.h"
#include "transform_component.h"

#include <gtest/gtest.h>
#include <cstdint>
#include <string>

TEST(EntityEditingRegressionTest, CreateAddsDefaultTransformNameAndSelectsEntity) {
    Scene scene("Test");
    EditorState state;

    const std::uint64_t id = createEntityAction(scene, state);

    ASSERT_EQ(scene.getEntities().size(), 1u);
    const Entity& created = *scene.getEntities().front();
    EXPECT_EQ(created.getId(), id);
    EXPECT_EQ(created.getName(), "Entity " + std::to_string(id));
    EXPECT_NE(scene.getEntities().front()->getComponent<TransformComponent>(), nullptr);
    EXPECT_EQ(state.getSelectedId(), id);
    EXPECT_TRUE(state.isDirty());
}

TEST(EntityEditingRegressionTest, DuplicateNamesAreAllowedAndIdsRemainDistinct) {
    Scene scene("Test");
    EditorState state;
    const auto firstId = createEntityAction(scene, state);
    const auto secondId = createEntityAction(scene, state);
    ASSERT_EQ(scene.getEntities().size(), 2u);
    ASSERT_NE(firstId, secondId);
    state.clearDirty();

    EXPECT_EQ(renameEntityAction(*scene.getEntities()[0], "  Coin \t", state),
              EditResult::Applied);
    EXPECT_EQ(renameEntityAction(*scene.getEntities()[1], "Coin", state),
              EditResult::Applied);
    EXPECT_EQ(scene.getEntities()[0]->getName(), "Coin");
    EXPECT_EQ(scene.getEntities()[1]->getName(), "Coin");
    EXPECT_TRUE(state.isDirty());
    EXPECT_EQ(state.getSelectedId(), secondId);
}

TEST(EntityEditingRegressionTest, InvalidRenameLeavesEntityAndDirtyFlagUnchanged) {
    Scene scene("Test");
    EditorState state;
    createEntityAction(scene, state);
    Entity& entity = *scene.getEntities().front();
    const std::string originalName = entity.getName();
    state.clearDirty();

    EXPECT_EQ(renameEntityAction(entity, " \r\t\n ", state), EditResult::Rejected);
    EXPECT_EQ(renameEntityAction(entity, originalName, state), EditResult::NoChange);
    EXPECT_EQ(renameEntityAction(entity, "  " + originalName + "  ", state),
              EditResult::NoChange);
    EXPECT_EQ(entity.getName(), originalName);
    EXPECT_FALSE(state.isDirty());
}

TEST(EntityEditingRegressionTest, RemovingEntityDoesNotReuseItsId) {
    Scene scene("Test");
    const std::uint64_t first = scene.createEntity().getId();
    const std::uint64_t second = scene.createEntity().getId();
    const std::uint64_t third = scene.createEntity().getId();

    ASSERT_TRUE(scene.removeEntity(second));
    const std::uint64_t fourth = scene.createEntity().getId();

    EXPECT_GT(fourth, third);
    ASSERT_EQ(scene.getEntities().size(), 3u);
    EXPECT_EQ(scene.getEntities()[0]->getId(), first);
    EXPECT_EQ(scene.getEntities()[1]->getId(), third);
    EXPECT_EQ(scene.getEntities()[2]->getId(), fourth);
}

TEST(EntityEditingRegressionTest, DeletingUnknownOrAlreadyRemovedEntityDoesNothing) {
    Scene scene("Test");
    const std::uint64_t existing = scene.createEntity().getId();

    EXPECT_FALSE(scene.removeEntity(9999));
    ASSERT_EQ(scene.getEntities().size(), 1u);
    EXPECT_EQ(scene.getEntities().front()->getId(), existing);

    EXPECT_TRUE(scene.removeEntity(existing));
    EXPECT_FALSE(scene.removeEntity(existing));
    EXPECT_TRUE(scene.getEntities().empty());
}

TEST(EntityEditingRegressionTest, DeleteRequestIsDeferredAndConsumedOnlyOnce) {
    Scene scene("Test");
    EditorState state;
    const auto id = createEntityAction(scene, state);
    state.clearDirty();

    state.requestDelete(id);
    EXPECT_EQ(scene.getEntities().size(), 1u)
        << "Requesting deletion must not change a scene during iteration";
    EXPECT_FALSE(state.isDirty());

    const auto requested = state.takePendingDelete();
    ASSERT_TRUE(requested.has_value());
    EXPECT_EQ(*requested, id);
    EXPECT_FALSE(state.takePendingDelete().has_value());
    EXPECT_EQ(scene.getEntities().size(), 1u);

    // The engine performs this removal at the safe frame boundary.
    EXPECT_TRUE(scene.removeEntity(*requested));
    EXPECT_TRUE(scene.getEntities().empty());
}

TEST(EntityEditingRegressionTest, SceneReplacementDiscardsOldSceneDeleteRequest) {
    EditorState state;
    state.select(8);
    state.requestDelete(8);
    state.onSceneReplaced();

    EXPECT_FALSE(state.takePendingDelete().has_value());
    EXPECT_FALSE(state.getSelectedId().has_value());
    EXPECT_EQ(state.getSceneGeneration(), 1u);
}

TEST(EntityEditingRegressionTest, CreatingAfterDeleteSelectsNewEntity) {
    Scene scene("Test");
    EditorState state;
    const auto oldId = createEntityAction(scene, state);
    ASSERT_TRUE(scene.removeEntity(oldId));
    state.unselect(); // Engine clears selection after successful pending deletion.
    state.clearDirty();

    const auto newId = createEntityAction(scene, state);
    EXPECT_GT(newId, oldId);
    EXPECT_EQ(state.getSelectedId(), newId);
    EXPECT_TRUE(state.isDirty());
    EXPECT_EQ(scene.getEntities().size(), 1u);
}
