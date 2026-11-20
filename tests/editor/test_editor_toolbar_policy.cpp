#include "editor_toolbar_policy.h"

#include <gtest/gtest.h>

TEST(EditorToolbarPolicyTest, FreshPausedEditorAllowsFileActionsAndPlayback) {
    EditorState state;
    EXPECT_TRUE(EditorToolbarPolicy::canUseSceneFiles(state, false));
    EXPECT_TRUE(EditorToolbarPolicy::canTogglePlayback(state, false));
}

TEST(EditorToolbarPolicyTest, RunningEditorBlocksFileActionsButCanPause) {
    EditorState state;
    state.resume();
    EXPECT_FALSE(EditorToolbarPolicy::canUseSceneFiles(state, false));
    EXPECT_TRUE(EditorToolbarPolicy::canTogglePlayback(state, false));
}

TEST(EditorToolbarPolicyTest, OpenDialogBlocksBothActions) {
    EditorState state;
    EXPECT_FALSE(EditorToolbarPolicy::canUseSceneFiles(state, true));
    EXPECT_FALSE(EditorToolbarPolicy::canTogglePlayback(state, true));
}

TEST(EditorToolbarPolicyTest, PendingOpenBlocksBothActions) {
    EditorState state;
    state.setPendingOpen("scenes/next.json");
    EXPECT_FALSE(EditorToolbarPolicy::canUseSceneFiles(state, false));
    EXPECT_FALSE(EditorToolbarPolicy::canTogglePlayback(state, false));
}

TEST(EditorToolbarPolicyTest, PendingCloseBlocksBothActions) {
    EditorState state;
    state.setPendingClose();
    EXPECT_FALSE(EditorToolbarPolicy::canUseSceneFiles(state, false));
    EXPECT_FALSE(EditorToolbarPolicy::canTogglePlayback(state, false));
}

TEST(EditorToolbarPolicyTest, CancellingPendingActionRestoresAccess) {
    EditorState state;
    state.setPendingOpen("scenes/next.json");
    state.clearPendingAction();
    EXPECT_TRUE(EditorToolbarPolicy::canUseSceneFiles(state, false));
    EXPECT_TRUE(EditorToolbarPolicy::canTogglePlayback(state, false));
}
