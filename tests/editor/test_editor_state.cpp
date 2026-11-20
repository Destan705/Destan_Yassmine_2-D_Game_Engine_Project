#include "editor_state.h"
#include "scene.h"
#include "transform_component.h"
#include <gtest/gtest.h>

TEST(EditorStateTest, StartsPausedWithNoSelectionAndCleanScene) {
    EditorState state;
    EXPECT_TRUE(state.getPause());
    EXPECT_FALSE(state.getSelectedId().has_value());
    EXPECT_EQ(state.getSceneGeneration(), 0u);
    EXPECT_FALSE(state.isDirty());
    EXPECT_FALSE(state.getCurrentScenePath().has_value());
}

TEST(EditorStateTest, SelectionIsIdNotRetainedPointer) {
    EditorState state;
    Scene scene("First");
    auto& entity = scene.createEntity();
    entity.addComponent<TransformComponent>();
    state.select(entity.getId());
    ASSERT_TRUE(state.getSelectedId().has_value());
    EXPECT_EQ(*state.getSelectedId(), entity.getId());
    state.unselect();
    EXPECT_FALSE(state.getSelectedId().has_value());
}

TEST(EditorStateTest, SameNameAndReusedIdStillClearSelection) {
    EditorState state;
    Scene first("Same Name");
    Scene second("Same Name");
    EXPECT_EQ(first.createEntity().getId(), second.createEntity().getId());
    state.select(first.getEntities()[0]->getId());
    state.onSceneReplaced();
    EXPECT_FALSE(state.getSelectedId().has_value());
    EXPECT_EQ(state.getSceneGeneration(), 1u);
    state.select(second.getEntities()[0]->getId());
    state.onSceneReplaced();
    EXPECT_FALSE(state.getSelectedId().has_value());
    EXPECT_EQ(state.getSceneGeneration(), 2u);
}

TEST(EditorStateTest, MissingIdCanBeDetectedAgainstActiveScene) {
    EditorState state;
    Scene scene("Empty");
    state.select(100);
    bool exists = false;
    for (const auto& entity : scene.getEntities()) {
        if (entity->getId() == state.getSelectedId()) exists = true;
    }
    EXPECT_FALSE(exists);
}

TEST(EditorStateTest, ResumeAndPauseDoNotMarkCleanSceneDirty) {
    EditorState state;
    state.resume();
    EXPECT_FALSE(state.getPause());
    EXPECT_FALSE(state.isDirty());
    state.pause();
    EXPECT_TRUE(state.getPause());
    EXPECT_FALSE(state.isDirty());
}

TEST(EditorStateTest, DirtyAndPathOperationsAreIndependent) {
    EditorState state;
    state.setCurrentScenePath("scenes/example.json");
    EXPECT_EQ(state.getCurrentScenePath()->generic_string(), "scenes/example.json");
    EXPECT_FALSE(state.isDirty());
    state.markDirty();
    EXPECT_TRUE(state.isDirty());
    state.clearDirty();
    EXPECT_FALSE(state.isDirty());
    state.clearCurrentScenePath();
    EXPECT_FALSE(state.getCurrentScenePath().has_value());
}

TEST(EditorStateTest, SceneReplacementDropsStaleCollisionsAndDeleteRequest) {
    EditorState state;
    state.select(1);
    state.setCollisions({{1, 2}});
    state.requestDelete(1);
    state.onSceneReplaced();
    EXPECT_TRUE(state.getCollisions().empty());
    EXPECT_FALSE(state.getSelectedId().has_value());
    EXPECT_FALSE(state.takePendingDelete().has_value());
}

TEST(EditorStateTest, DeleteRequestIsConsumedExactlyOnce) {
    EditorState state;
    state.requestDelete(42);
    ASSERT_TRUE(state.takePendingDelete().has_value());
    EXPECT_FALSE(state.takePendingDelete().has_value());
}

TEST(EditorStateTest, PendingOpenAndCloseActionsCanBeCleared) {
    EditorState state;
    state.setPendingOpen("scenes/other.json");
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::Open);
    EXPECT_EQ(state.getPendingOpenPath()->generic_string(), "scenes/other.json");
    state.setPendingClose();
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::Close);
    EXPECT_FALSE(state.getPendingOpenPath().has_value());
    state.setAwaitingSavePath(true);
    state.clearPendingAction();
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::None);
    EXPECT_FALSE(state.isAwaitingSavePath());
}
