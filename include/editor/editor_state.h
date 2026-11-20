#pragma once
#include <cstdint>
#include <optional>
#include <filesystem>

enum class PendingSceneAction {
  None,
  Open,
  Close
};

// Shared editor information that every panel reads.
// Plain C++ (no ImGui), so it is easy to unit-test.
class EditorState {

  public:
    // Selects the entity with this ID (e.g. when it's clicked in the Hierarchy).
    void select(std::uint64_t entityId);

    // Clears the selection, so nothing is selected.
    void unselect();

    // Returns the selected entity's ID, or nothing (std::nullopt) if none is selected.
    // The caller looks the ID up in the active Scene; we never keep an Entity pointer.
    std::optional<std::uint64_t> getSelectedId() const;

    // Call after a new Scene replaces the old one.
    // Clears the selection and increases the scene generation by 1.
    void onSceneReplaced();

    // Returns how many times the scene has been replaced.
    // Panels compare it to notice a scene change (e.g. to reset half-typed values).
    std::uint64_t getSceneGeneration() const;

    // Returns true if the game is paused.
    bool getPause() const;

    // Pauses the game: no gameplay updates or gameplay sounds.
    void pause();

    // Resumes the game.
    void resume();

    // return whether the scene is dirty
    bool isDirty() const;

    // marks a scene as dirty
    void markDirty();

    // unmarks the scene as dirty
    void clearDirty(); 

    std::optional<std::filesystem::path> getCurrentScenePath() const;

    void setCurrentScenePath(std::filesystem::path path);

    void clearCurrentScenePath();

    PendingSceneAction getPendingAction() const;

    std::optional<std::filesystem::path> getPendingOpenPath() const;

    void setPendingOpen(const std::filesystem::path& path);

    void setPendingClose();

    void clearPendingAction();

    bool isAwaitingSavePath() const;

    void setAwaitingSavePath(bool awaiting);


    void requestDelete(std::uint64_t id);

    std::optional<std::uint64_t> takePendingDelete();
  private:
    // ID of the selected entity, or empty if nothing is selected.
    std::optional<std::uint64_t> selectId;

    // Scene "version": goes up by 1 every time the scene is replaced.
    std::uint64_t sceneGeneration = 0;

    // Editor sessions start paused, so the game doesn't overwrite your edits.
    bool paused = true;

    //wether changes in a scene is saved or not
    bool dirty = false;

    std::optional<std::uint64_t> pendingDeleteId;

    // Current path != path user is typing into the panel 
    std::optional<std::filesystem::path> currentScenePath = std::nullopt;

    PendingSceneAction pendingAction = PendingSceneAction::None;

    std::optional<std::filesystem::path> pendingOpenPath = std::nullopt;

    bool awaitingSavePath = false;
};
