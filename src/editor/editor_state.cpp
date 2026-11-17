#include "editor_state.h"
#include <utility>

// Entity selection.

void EditorState::select(std::uint64_t entityId) {
    selectId = entityId;
}

void EditorState::unselect() {
    selectId = std::nullopt;
}

std::optional<std::uint64_t> EditorState::getSelectedId() const {
    return selectId;
}


// Scene replacement.

void EditorState::onSceneReplaced() {
    unselect();
    collisions.clear();
    sceneGeneration++;
}

std::uint64_t EditorState::getSceneGeneration() const {
    return sceneGeneration;
}


// Simulation state.

bool EditorState::getPause() const {
    return paused;
}

void EditorState::pause() {
    paused = true;
}

void EditorState::resume() {
    paused = false;
    markDirty();
}


// Dirty state.

bool EditorState::isDirty() const {
    return dirty;
}

void EditorState::markDirty() {
    dirty = true;
}

void EditorState::clearDirty() {
    dirty = false;
}


// Current scene file path.

std::optional<std::filesystem::path> EditorState::getCurrentScenePath() const {
    return currentScenePath;
}

void EditorState::setCurrentScenePath(std::filesystem::path path) {
    currentScenePath = std::move(path);
}

void EditorState::clearCurrentScenePath() {
    currentScenePath.reset();
}


// Pending scene file operations.

PendingSceneAction EditorState::getPendingAction() const {
    return pendingAction;
}

std::optional<std::filesystem::path> EditorState::getPendingOpenPath() const {
    return pendingOpenPath;
}

void EditorState::setPendingOpen(const std::filesystem::path& path) {
    pendingOpenPath = path;
    pendingAction = PendingSceneAction::Open;
}

void EditorState::setPendingClose() {
    pendingAction = PendingSceneAction::Close;
    pendingOpenPath = std::nullopt;
}

void EditorState::clearPendingAction() {
    pendingAction = PendingSceneAction::None;
    pendingOpenPath = std::nullopt;
    awaitingSavePath = false;
}


// Save path popup state.

bool EditorState::isAwaitingSavePath() const {
    return awaitingSavePath;
}

void EditorState::setAwaitingSavePath(bool awaiting) {
    awaitingSavePath = awaiting;
}


// Collision snapshot.

const std::vector<std::pair<std::uint64_t, std::uint64_t>>& EditorState::getCollisions() const {
    return collisions;
}

void EditorState::setCollisions(const std::vector<std::pair<std::uint64_t, std::uint64_t>>& collisionList) {
    collisions = collisionList;
}


// Collider overlay visibility.

bool EditorState::getShowColliders() const {
    return showColliders;
}

void EditorState::setShowColliders(const bool displayColliders) {
    showColliders = displayColliders;
}