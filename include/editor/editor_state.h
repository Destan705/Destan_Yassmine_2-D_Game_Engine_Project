#pragma once

#include <cstdint>
#include <optional>
#include <filesystem>
#include <vector>
#include <utility>

enum class PendingSceneAction {
    None,
    Open,
    Close
};

// Shared editor information that every panel reads.
// Plain C++ (no ImGui), so it is easy to unit-test.
class EditorState {

public:
    // Entity selection.
    void select(std::uint64_t entityId);
    void unselect();
    std::optional<std::uint64_t> getSelectedId() const;

    // Scene replacement.
    void onSceneReplaced();
    std::uint64_t getSceneGeneration() const;

    // Simulation state.
    bool getPause() const;
    void pause();
    void resume();

    // Dirty state.
    bool isDirty() const;
    void markDirty();
    void clearDirty();

    // Current scene file path.
    std::optional<std::filesystem::path> getCurrentScenePath() const;
    void setCurrentScenePath(std::filesystem::path path);
    void clearCurrentScenePath();

    // Pending scene file operations.
    PendingSceneAction getPendingAction() const;
    std::optional<std::filesystem::path> getPendingOpenPath() const;
    void setPendingOpen(const std::filesystem::path& path);
    void setPendingClose();
    void clearPendingAction();

    // Save path popup state.
    bool isAwaitingSavePath() const;
    void setAwaitingSavePath(bool awaiting);


    void requestDelete(std::uint64_t id);

    std::optional<std::uint64_t> takePendingDelete();
    // Current collision snapshot.
    const std::vector<std::pair<std::uint64_t, std::uint64_t>>& getCollisions() const;
    void setCollisions(const std::vector<std::pair<std::uint64_t, std::uint64_t>>& collisionList);

    // Collider overlay visibility.
    bool getShowColliders() const;
    void setShowColliders(const bool displayColliders);

private:
    // Entity selection and scene generation.
    std::optional<std::uint64_t> selectId;
    std::uint64_t sceneGeneration = 0;

    // Simulation and dirty state.
    bool paused = true;
    bool dirty = false;

    std::optional<std::uint64_t> pendingDeleteId;
    
    // Scene file management.
    std::optional<std::filesystem::path> currentScenePath = std::nullopt;
    PendingSceneAction pendingAction = PendingSceneAction::None;
    std::optional<std::filesystem::path> pendingOpenPath = std::nullopt;
    bool awaitingSavePath = false;

    // Collider overlay state.
    std::vector<std::pair<std::uint64_t, std::uint64_t>> collisions;
    bool showColliders = true;
};