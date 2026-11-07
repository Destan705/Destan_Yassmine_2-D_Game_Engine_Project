#pragma once
#include <cstdint>
#include <optional>

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

  private:
    // ID of the selected entity, or empty if nothing is selected.
    std::optional<std::uint64_t> selectId;

    // Scene "version": goes up by 1 every time the scene is replaced.
    std::uint64_t sceneGeneration = 0;

    // Editor sessions start paused, so the game doesn't overwrite your edits.
    bool paused = true;
};