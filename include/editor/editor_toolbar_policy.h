#pragma once

#include "editor_state.h"

// Keep the toolbar and File menu consistent with the existing scene-file
// transaction rules. This header has no ImGui dependency and is unit-testable.
namespace EditorToolbarPolicy {
inline bool canUseSceneFiles(const EditorState& state, bool dialogOpen) {
    return state.getPause() &&
           state.getPendingAction() == PendingSceneAction::None &&
           !dialogOpen;
}

inline bool canTogglePlayback(const EditorState& state, bool dialogOpen) {
    return state.getPendingAction() == PendingSceneAction::None && !dialogOpen;
}
} // namespace EditorToolbarPolicy
