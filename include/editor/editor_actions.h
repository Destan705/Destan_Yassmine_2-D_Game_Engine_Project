#pragma once
#include "entity.h"
#include <string>
#include <filesystem>

class AssetManager;
class SceneManager;
class SceneSerializer;
class EditorState;

enum class EditResult{

Applied,
NoChange,
Rejected

};

enum class UnsavedChangesChoice{
    Save,
    Discard,
    Cancel
};

enum class SceneRequestStatus{
    Opened,
    AwaitingConfirmation,
    ReadyToClose,
    Rejected,
    Cancelled,
    AwaitingSavePath
};

struct SceneFileResult{
    bool success;
    std::string message;
};

struct SceneRequestResult{
    SceneRequestStatus status;
    std::string message;
};

EditResult applyTransformEdit(Entity& entity, float x, float y, float width, float height);

EditResult applyColliderEdit(Entity& entity, float offsetX, float offsetY, float width, float height, bool enabled);

EditResult applySpriteEdit(Entity& entity, AssetManager& assetManager, const std::string& relativePath);

std::filesystem::path normalizeScenePath(const std::string& relativePath);

std::filesystem::path resolveScenePath(const std::filesystem::path& assetRoot, const std::string& relativePath);

SceneFileResult openScene(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath);

SceneFileResult saveSceneAs(SceneManager& sceneManager, SceneSerializer& sceneSerializer, const AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath);

SceneFileResult saveScene(SceneManager& sceneManager, SceneSerializer& sceneSerializer, const AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot);

SceneRequestResult requestOpen(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath);

SceneRequestResult requestClose(EditorState& editorState);

SceneRequestResult executePendingAction(SceneManager& sceneManager, SceneSerializer& sceneSerializer, const AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot);

SceneRequestResult resolveUnsavedChanges(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, UnsavedChangesChoice choice);

SceneRequestResult provideSavePath(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath);