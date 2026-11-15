#include "editor_actions.h"
#include "transform_component.h"
#include "collider_component.h"
#include "sprite_component.h"
#include "animation_component.h"
#include "asset_manager.h"
#include "editor_state.h"
#include "scene_manager.h"
#include "scene_serializer.h"
#include "scene.h"
#include <cmath>
#include <exception>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <filesystem>
#include <memory>
#include <string>


EditResult applyTransformEdit(Entity& entity, float x, float y, float width, float height) {

    TransformComponent* transform = entity.getComponent<TransformComponent>();

    if (transform == nullptr || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) || width <= 0 || !std::isfinite(height) || height <= 0) {
        return EditResult::Rejected;
    }

    if (transform->x == x && transform->y == y && transform->width == width && transform->height == height) {
        return EditResult::NoChange;
    }

    transform->x = x;
    transform->y = y;
    transform->width = width;
    transform->height = height;

    return EditResult::Applied;
}


EditResult applyColliderEdit(Entity& entity, float offsetX, float offsetY, float width, float height, bool enabled) {

    ColliderComponent* collider = entity.getComponent<ColliderComponent>();

    if (collider == nullptr || !std::isfinite(offsetX) || !std::isfinite(offsetY) || !std::isfinite(width) || width <= 0 || !std::isfinite(height) || height <= 0) {
        return EditResult::Rejected;
    }

    if (collider->getOffsetX() == offsetX && collider->getOffsetY() == offsetY && collider->getWidth() == width && collider->getHeight() == height && collider->isEnabled() == enabled) {
        return EditResult::NoChange;
    }

    collider->setLocalBounds(offsetX, offsetY, width, height);
    collider->setEnabled(enabled);

    return EditResult::Applied;
}


EditResult applySpriteEdit(Entity& entity, AssetManager& assetManager, const std::string& relativePath) {

    SpriteComponent* sprite = entity.getComponent<SpriteComponent>();

    if (sprite == nullptr) {
        return EditResult::Rejected;
    }

    try {
        textureHandle candidateTexture = assetManager.loadTexture(relativePath);

        if (sprite->getTexture() == candidateTexture) {
            return EditResult::NoChange;
        }

        AnimationComponent* animation = entity.getComponent<AnimationComponent>();

        std::pair<int, int> originalDimensions = assetManager.getOriginalDimensions(candidateTexture);

        if (originalDimensions.first <= 0 || originalDimensions.second <= 0) {
            return EditResult::Rejected;
        }

        if (animation != nullptr) {
            for (const auto& frame : animation->getFrames()) {
                bool checkFinite = std::isfinite(frame.x) && std::isfinite(frame.y) && std::isfinite(frame.w) && std::isfinite(frame.h);
                bool checkBounds = frame.x >= 0 && frame.y >= 0 && frame.w > 0 && frame.h > 0;
                bool checkBoundaries = (frame.x + frame.w <= originalDimensions.first) && (frame.y + frame.h <= originalDimensions.second);

                if (!checkFinite || !checkBounds || !checkBoundaries) {
                    return EditResult::Rejected;
                }
            }
        }

        sprite->setTexture(candidateTexture);

        return EditResult::Applied;

    } catch (const std::exception&) {
        return EditResult::Rejected;
    }
}


// Normalizes and validates an asset-relative scene path.
std::filesystem::path normalizeScenePath(const std::string& relativePath) {

    // Standardize path separators.
    std::string stringPath = relativePath;
    std::replace(stringPath.begin(), stringPath.end(), '\\', '/');

    // Reject invalid paths.
    if (stringPath.empty()) {
        throw std::invalid_argument("Empty paths are not allowed");
    }

    if (stringPath.find('\0') != std::string::npos) {
        throw std::invalid_argument("Null characters are not allowed in paths");
    }

    if (stringPath.find(':') != std::string::npos) {
        throw std::invalid_argument("Colons are not allowed in paths");
    }

    std::filesystem::path standardizedPath = stringPath;

    if (standardizedPath.is_absolute()) {
        throw std::invalid_argument("Absolute paths are not allowed");
    }

    if (stringPath.compare(0, 2, "//") == 0 || stringPath[0] == '/') {
        throw std::invalid_argument("Absolute or rooted paths are not allowed");
    }

    for (const auto& component : standardizedPath) {
        if (component == "..") {
            throw std::invalid_argument(".. traversals in paths are not allowed");
        }
    }

    std::filesystem::path lexicallyNormalPath = standardizedPath.lexically_normal();

    if (lexicallyNormalPath.empty() || lexicallyNormalPath == ".") {
        throw std::invalid_argument("Empty paths are not allowed");
    }

    auto iterator = lexicallyNormalPath.begin();

    if (*iterator != "scenes") {
        throw std::invalid_argument("Paths must be in the scenes directory");
    }

    if (lexicallyNormalPath.extension() != ".json") {
        throw std::invalid_argument("Only JSON scene files are allowed");
    }

    return lexicallyNormalPath;
}


// Resolves a relative scene path while ensuring it stays in the scenes directory.
std::filesystem::path resolveScenePath(const std::filesystem::path& assetRoot, const std::string& relativePath) {

    std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);
    std::filesystem::path canonicalRoot = std::filesystem::canonical(assetRoot);
    std::filesystem::path fullPath = canonicalRoot / normalizedScenePath;
    std::filesystem::path resolvedPath = std::filesystem::weakly_canonical(fullPath);
    std::filesystem::path expectedDirectory = canonicalRoot / "scenes";

    auto expectedIterator = expectedDirectory.begin();
    auto resolvedIterator = resolvedPath.begin();

    while (expectedIterator != expectedDirectory.end()) {
        if (resolvedIterator == resolvedPath.end() || *resolvedIterator != *expectedIterator) {
            throw std::invalid_argument("Path escapes the scenes directory");
        }

        expectedIterator++;
        resolvedIterator++;
    }

    if (resolvedIterator == resolvedPath.end()) {
        throw std::invalid_argument("Path must point to a scene file");
    }

    return resolvedPath;
}


// Loads a replacement scene before replacing the active scene.
SceneFileResult openScene(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath) {

    if (!editorState.getPause()) {
        return {false, "Engine must be paused in order to open a scene"};
    }

    try {
        std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);
        std::filesystem::path resolvedScenePath = resolveScenePath(assetRoot, relativePath);

        // Fully load the candidate before replacing the current scene.
        std::unique_ptr<Scene> candidateScene = sceneSerializer.load(resolvedScenePath, assetManager);

        candidateScene->update(0.0f);

        sceneManager.setActiveScene(std::move(candidateScene));

        // Reset editor state only after successful activation.
        editorState.onSceneReplaced();
        editorState.setCurrentScenePath(std::move(normalizedScenePath));
        editorState.clearDirty();

    } catch (const std::exception& e) {
        return {false, e.what()};
    }

    return {true, "Scene opened successfully"};
}


// Saves the active scene to a specified path.
SceneFileResult saveSceneAs(SceneManager& sceneManager, SceneSerializer& sceneSerializer, const AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath) {

    if (!editorState.getPause()) {
        return {false, "Engine must be paused in order to save a scene"};
    }

    try {
        std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);
        std::filesystem::path resolvedScenePath = resolveScenePath(assetRoot, relativePath);

        const Scene& activeScene = sceneManager.getActiveScene();

        sceneSerializer.save(activeScene, resolvedScenePath, assetManager);

        // Update editor state only after saving succeeds.
        editorState.setCurrentScenePath(std::move(normalizedScenePath));
        editorState.clearDirty();

    } catch (const std::exception& e) {
        return {false, e.what()};
    }

    return {true, "Scene saved successfully"};
}


// Saves the active scene to its current associated path.
SceneFileResult saveScene(SceneManager& sceneManager, SceneSerializer& sceneSerializer, const AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot) {

    if (!editorState.getPause()) {
        return {false, "Engine must be paused in order to save a scene"};
    }

    try {
        auto currentScenePath = editorState.getCurrentScenePath();

        if (!currentScenePath.has_value()) {
            return {false, "Scene requires a save location"};
        }

        return saveSceneAs(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, currentScenePath->generic_string());

    } catch (const std::exception& e) {
        return {false, e.what()};
    }
}


// Requests to open a scene, prompting for confirmation if necessary.
SceneRequestResult requestOpen(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath) {

    if (!editorState.getPause()) {
        return {SceneRequestStatus::Rejected, "Engine must be paused in order to open a scene"};
    }

    if (editorState.getPendingAction() != PendingSceneAction::None) {
        return {SceneRequestStatus::Rejected, "Another scene action is already pending"};
    }

    try {
        std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);

        // Validate that the destination stays inside the scenes directory.
        resolveScenePath(assetRoot, relativePath);

        if (editorState.isDirty()) {
            editorState.setPendingOpen(normalizedScenePath);

            return {SceneRequestStatus::AwaitingConfirmation, "You have unsaved changes. Save before opening another scene?"};
        }

        SceneFileResult openSceneResult = openScene(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, relativePath);

        if (openSceneResult.success) {
            return {SceneRequestStatus::Opened, openSceneResult.message};
        }

        return {SceneRequestStatus::Rejected, openSceneResult.message};

    } catch (const std::exception& e) {
        return {SceneRequestStatus::Rejected, std::string("Opening scene was unsuccessful: ") + e.what()};
    }
}


// Requests application closure, prompting for confirmation if necessary.
SceneRequestResult requestClose(EditorState& editorState) {

    if (editorState.getPendingAction() != PendingSceneAction::None) {
        return {SceneRequestStatus::Rejected, "Another scene action is already pending"};
    }

    if (editorState.isDirty()) {
        editorState.pause();
        editorState.setPendingClose();

        return {SceneRequestStatus::AwaitingConfirmation, "You have unsaved changes. Save before closing?"};
    }

    return {SceneRequestStatus::ReadyToClose, "Application can be closed"};
}


// Executes a pending action after unsaved changes have been resolved.
SceneRequestResult executePendingAction(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot) {

    if (!editorState.getPause()) {
        return {SceneRequestStatus::Rejected, "Engine must be paused to execute a pending action"};
    }

    try {
        PendingSceneAction pendingAction = editorState.getPendingAction();
        auto pendingOpenPath = editorState.getPendingOpenPath();

        // Consume the pending action after retrieving its information.
        editorState.clearPendingAction();

        switch (pendingAction) {

            case PendingSceneAction::Open: {
                if (!pendingOpenPath.has_value()) {
                    return {SceneRequestStatus::Rejected, "Open path is missing"};
                }

                SceneFileResult openSceneResult = openScene(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, pendingOpenPath->generic_string());

                if (openSceneResult.success) {
                    return {SceneRequestStatus::Opened, openSceneResult.message};
                }

                return {SceneRequestStatus::Rejected, openSceneResult.message};
            }

            case PendingSceneAction::Close:
                return {SceneRequestStatus::ReadyToClose, "Application can be closed"};

            case PendingSceneAction::None:
                return {SceneRequestStatus::Rejected, "Nothing to execute"};
        }

        return {SceneRequestStatus::Rejected, "Unknown pending action"};

    } catch (const std::exception& e) {
        editorState.clearPendingAction();

        return {SceneRequestStatus::Rejected, e.what()};
    }
}


// Handles the user's Save, Discard or Cancel decision.
SceneRequestResult resolveUnsavedChanges(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, UnsavedChangesChoice choice) {

    if (!editorState.getPause()) {
        return {SceneRequestStatus::Rejected, "Engine must be paused to resolve unsaved changes"};
    }

    PendingSceneAction pendingAction = editorState.getPendingAction();

    if (pendingAction == PendingSceneAction::None) {
        return {SceneRequestStatus::Rejected, "No pending action to resolve"};
    }

    switch (choice) {

        case UnsavedChangesChoice::Cancel:
            editorState.clearPendingAction();

            return {SceneRequestStatus::Cancelled, "Pending operation cancelled"};

        case UnsavedChangesChoice::Discard:
            return executePendingAction(sceneManager, sceneSerializer, assetManager, editorState, assetRoot);

        case UnsavedChangesChoice::Save: {

            // An untitled scene requires a save destination.
            if (!editorState.getCurrentScenePath().has_value()) {
                editorState.setAwaitingSavePath(true);

                return {SceneRequestStatus::AwaitingSavePath, "Please provide a save location"};
            }

            SceneFileResult saveResult = saveScene(sceneManager, sceneSerializer, assetManager, editorState, assetRoot);

            // Only execute the pending action if saving succeeds.
            if (saveResult.success) {
                return executePendingAction(sceneManager, sceneSerializer, assetManager, editorState, assetRoot);
            }

            editorState.clearPendingAction();

            return {SceneRequestStatus::Rejected, saveResult.message};
        }
    }

    return {SceneRequestStatus::Rejected, "Unknown unsaved changes choice"};
}


// Provides a save location for an untitled scene before executing a pending Open or Close action.
SceneRequestResult provideSavePath(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath) {

    if (!editorState.getPause()) {
        return {SceneRequestStatus::Rejected, "Engine must be paused to provide a save path"};
    }

    PendingSceneAction pendingAction = editorState.getPendingAction();

    if (pendingAction == PendingSceneAction::None) {
        return {SceneRequestStatus::Rejected, "No pending action to resolve"};
    }

    if (!editorState.isAwaitingSavePath()) {
        return {SceneRequestStatus::Rejected, "Not awaiting a save path"};
    }

    try {
        std::filesystem::path resolvedSavePath = resolveScenePath(assetRoot, relativePath);

        // Prevent overwriting the scene we are about to open.
        if (pendingAction == PendingSceneAction::Open) {

            auto pendingOpenPath = editorState.getPendingOpenPath();

            if (!pendingOpenPath.has_value()) {
                editorState.clearPendingAction();

                return {SceneRequestStatus::Rejected, "Open path is missing"};
            }

            std::filesystem::path resolvedOpenPath = resolveScenePath(assetRoot, pendingOpenPath->generic_string());

            if (resolvedSavePath == resolvedOpenPath || (std::filesystem::exists(resolvedOpenPath) && std::filesystem::exists(resolvedSavePath) && std::filesystem::equivalent(resolvedSavePath, resolvedOpenPath))) {

                editorState.clearPendingAction();

                return {SceneRequestStatus::Rejected, "Save destination is the same file we're trying to open"};
            }
        }

        // Save the current scene before proceeding.
        SceneFileResult saveResult = saveSceneAs(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, relativePath);

        // If saving fails, cancel the pending operation.
        if (!saveResult.success) {
            editorState.clearPendingAction();

            return {SceneRequestStatus::Rejected, saveResult.message};
        }

        return executePendingAction(sceneManager, sceneSerializer, assetManager, editorState, assetRoot);

    } catch (const std::exception& e) {
        editorState.clearPendingAction();

        return {SceneRequestStatus::Rejected, std::string("Saving file was unsuccessful: ") + e.what()};
    }
}