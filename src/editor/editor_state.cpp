#include "editor_state.h"
#include <utility>

void EditorState::select(std::uint64_t entityId) { selectId = entityId; }
void EditorState::unselect() { selectId.reset(); }
std::optional<std::uint64_t> EditorState::getSelectedId() const { return selectId; }

void EditorState::requestDelete(std::uint64_t id) { pendingDeleteId = id; }
std::optional<std::uint64_t> EditorState::takePendingDelete() {
    auto request = pendingDeleteId;
    pendingDeleteId.reset();
    return request;
}

void EditorState::onSceneReplaced() {
    unselect();
    pendingDeleteId.reset(); // A pending request belongs to the previous scene only.
    collisions.clear();
    ++sceneGeneration;
}
std::uint64_t EditorState::getSceneGeneration() const { return sceneGeneration; }

bool EditorState::getPause() const { return paused; }
void EditorState::pause() { paused = true; }
void EditorState::resume() {
    paused = false;
    // Changing playback state does not itself edit saved scene data.
}

bool EditorState::isDirty() const { return dirty; }
void EditorState::markDirty() { dirty = true; }
void EditorState::clearDirty() { dirty = false; }

std::optional<std::filesystem::path> EditorState::getCurrentScenePath() const { return currentScenePath; }
void EditorState::setCurrentScenePath(std::filesystem::path path) { currentScenePath = std::move(path); }
void EditorState::clearCurrentScenePath() { currentScenePath.reset(); }

PendingSceneAction EditorState::getPendingAction() const { return pendingAction; }
std::optional<std::filesystem::path> EditorState::getPendingOpenPath() const { return pendingOpenPath; }
void EditorState::setPendingOpen(const std::filesystem::path& path) {
    pendingOpenPath = path;
    pendingAction = PendingSceneAction::Open;
}
void EditorState::setPendingClose() {
    pendingAction = PendingSceneAction::Close;
    pendingOpenPath.reset();
}
void EditorState::clearPendingAction() {
    pendingAction = PendingSceneAction::None;
    pendingOpenPath.reset();
    awaitingSavePath = false;
}

bool EditorState::isAwaitingSavePath() const { return awaitingSavePath; }
void EditorState::setAwaitingSavePath(bool awaiting) { awaitingSavePath = awaiting; }

const std::vector<std::pair<std::uint64_t, std::uint64_t>>& EditorState::getCollisions() const {
    return collisions;
}
void EditorState::setCollisions(const std::vector<std::pair<std::uint64_t, std::uint64_t>>& list) {
    collisions = list;
}
bool EditorState::getShowColliders() const { return showColliders; }
void EditorState::setShowColliders(const bool visible) { showColliders = visible; }
