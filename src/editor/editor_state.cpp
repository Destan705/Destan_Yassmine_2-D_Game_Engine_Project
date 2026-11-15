#include "editor_state.h"
#include <utility>

    // Selects the entity with this ID (e.g. when it's clicked in the Hierarchy).
    void EditorState::select(std::uint64_t entityId){
        selectId = entityId;
    }

    // Clears the selection, so nothing is selected.
    void EditorState::unselect(){
        selectId = std::nullopt;
    }

    // Returns the selected entity's ID, or nothing (std::nullopt) if none is selected.
    // The caller looks the ID up in the active Scene; we never keep an Entity pointer.
    std::optional<std::uint64_t> EditorState::getSelectedId() const{
        return selectId;
    }

    // Call after a new Scene replaces the old one.
    // Clears the selection and increases the scene generation by 1.
    void  EditorState::onSceneReplaced(){
        unselect();
        sceneGeneration++;
        
    }

    // Returns how many times the scene has been replaced.
    // Panels compare it to notice a scene change (e.g. to reset half-typed values).
    std::uint64_t EditorState::getSceneGeneration() const{
        return sceneGeneration;
    }

    // Returns true if the game is paused.
    bool EditorState::getPause() const{
        return paused;
    }

    // Pauses the game: no gameplay updates or gameplay sounds.
    void EditorState::pause(){
        paused =true;
    }

    // Resumes the game.
    void EditorState::resume(){
        paused=false;
        markDirty();
    }

        // return whether the scene is dirty
    bool EditorState::isDirty() const{
        return dirty;
    }

    // marks a scene as dirty
    void EditorState::markDirty(){
        dirty=true;
    }

    // unmarks the scene as dirty
    void EditorState::clearDirty(){
        dirty = false;
    }


    std::optional<std::filesystem::path> EditorState::getCurrentScenePath() const {
        return currentScenePath;
    }

    void EditorState::setCurrentScenePath(std::filesystem::path path) {
        currentScenePath = std::move(path);
    }

    void EditorState::clearCurrentScenePath() {
        currentScenePath.reset();
    }

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
    }
