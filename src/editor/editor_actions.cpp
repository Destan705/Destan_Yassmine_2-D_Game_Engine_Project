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

EditResult applyTransformEdit(Entity& entity, float x, float y, float width, float height){

    TransformComponent* transform = entity.getComponent<TransformComponent>();

    if(transform == nullptr||!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(width) || width <= 0 || !std::isfinite(height) || height <= 0 ){
        return EditResult::Rejected;
    }
    
    if(transform->x == x&&transform->y == y&&transform->width == width&&transform->height == height){
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

    if (collider == nullptr ||  !std::isfinite(offsetX) || !std::isfinite(offsetY) || !std::isfinite(width) || width <= 0 || !std::isfinite(height) || height <= 0 ){
        return EditResult::Rejected;
    }

    if (collider->getOffsetX() == offsetX && collider->getOffsetY() == offsetY && collider->getWidth() == width && collider->getHeight() == height && collider->isEnabled() == enabled){
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

        if (originalDimensions.first <= 0 || originalDimensions.second <= 0){
            return EditResult::Rejected;
        }

        if (animation != nullptr){
            for (const auto& frame : animation->getFrames()) {
                bool checkFinite = std::isfinite(frame.x) && std::isfinite(frame.y) && std::isfinite(frame.w) && std::isfinite(frame.h);
                bool checkBounds = frame.x >= 0 && frame.y >= 0 && frame.w > 0 && frame.h > 0;
                bool checkBoundaries = (frame.x + frame.w <= originalDimensions.first) && (frame.y + frame.h  <= originalDimensions.second);

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

std::filesystem::path normalizeScenePath(const std::string& relativePath) {
    // Standardizing separators -> Only using "/" instead of "\"
    std::string stringPath = relativePath;
    std::replace(stringPath.begin(), stringPath.end(), '\\', '/');

    // Reject invalid paths
    if (stringPath.empty()){
        throw std::invalid_argument("Empty paths are not allowed");
    }

    if (stringPath.find('\0') != std::string::npos) {
        throw std::invalid_argument("Null characters are not allowed in paths");
    }

    if (stringPath.find(':') != std::string::npos) {
        throw std::invalid_argument("Colons are not allowed in paths");
    }

    std::filesystem::path standardizedPath = stringPath;
    if (standardizedPath.is_absolute()){
        throw std::invalid_argument("Absolute paths are not allowed");
    }

    if (stringPath.compare(0, 2, "//") == 0 || stringPath[0] == '/') {
        throw std::invalid_argument("Absolute or rooted paths are not allowed");
    }

    for (const auto& component : standardizedPath) {
        if (component == ".."){
            throw std::invalid_argument(".. traversals in paths are not allowed");
        }
    }

    std::filesystem::path lexicallyNormalPath = standardizedPath.lexically_normal();

    if (lexicallyNormalPath.empty() || lexicallyNormalPath == "."){
        throw std::invalid_argument("Empty paths are not allowed");
    }

    auto iterator = lexicallyNormalPath.begin();
    if (*iterator != "scenes"){
        throw std::invalid_argument("Paths must be in the scenes directory");
    }

    if (lexicallyNormalPath.extension() != ".json"){
        throw std::invalid_argument("Only json scene files are allowed");
    }

    return lexicallyNormalPath;
}

std::filesystem::path resolveScenePath(const std::filesystem::path& assetRoot, const std::string& relativePath) {
    std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);
    std::filesystem::path canonicalRoot = std::filesystem::canonical(assetRoot);
    std::filesystem::path fullPath = canonicalRoot / normalizedScenePath;
    std::filesystem::path resolvedPath = std::filesystem::weakly_canonical(fullPath);
    std::filesystem::path expectedDirectory = canonicalRoot / "scenes";


    auto expectedIterator = expectedDirectory.begin();
    auto resolvedIterator = resolvedPath.begin();

    while (expectedIterator != expectedDirectory.end()){
        if (resolvedIterator == resolvedPath.end() || *resolvedIterator != *expectedIterator){
            throw std::invalid_argument("Path escapes engine directory");
        }
        expectedIterator++;
        resolvedIterator++;
    }
    if (resolvedIterator == resolvedPath.end()){
        throw std::invalid_argument("Path escapes the directory");
    }
    return resolvedPath;
}

SceneFileResult openScene(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath) {
    if (!editorState.getPause()) {
        return {false, "Engine must be paused in order to open new scene"};
    }

    try {
    
    std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);
    std::filesystem::path resolvedScenePath = resolveScenePath(assetRoot, relativePath);

    std::unique_ptr<Scene> candidateScene = sceneSerializer.load(resolvedScenePath, assetManager);
    
    
    candidateScene->update(0.0f);

    sceneManager.setActiveScene(std::move(candidateScene));

    editorState.onSceneReplaced();
    editorState.setCurrentScenePath(std::move(normalizedScenePath));
    editorState.clearDirty();

    } catch (const std::exception& e) {
            return {false, e.what()};
    }

    return {true, "Scene opened successfully"};
}

SceneFileResult saveSceneAs(SceneManager& sceneManager, SceneSerializer& sceneSerializer, const AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath) {
    if (!editorState.getPause()) {
        return {false, "Engine must be paused in order to save scene"};
    }

    try {
    
        std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);
        std::filesystem::path resolvedScenePath = resolveScenePath(assetRoot, relativePath);

        const Scene& activeScene = sceneManager.getActiveScene();

        sceneSerializer.save(activeScene, resolvedScenePath, assetManager);

        editorState.setCurrentScenePath(std::move(normalizedScenePath));
        editorState.clearDirty();

    } catch (const std::exception& e) {
        return {false, e.what()};
    }

    return {true, "Scene saved successfully"};
}


SceneFileResult saveScene(SceneManager& sceneManager, SceneSerializer& sceneSerializer, const AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot) {
    if (!editorState.getPause()) {
        return {false, "Engine must be paused in order to save scene"};
    }

    auto currentScenePath = editorState.getCurrentScenePath();

    if (!currentScenePath.has_value()) {
        return {false, "Save unsuccessful since scene requires a save location"};
    }

    return saveSceneAs(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, currentScenePath->generic_string());
}

SceneRequestResult requestOpen(SceneManager& sceneManager, SceneSerializer& sceneSerializer, AssetManager& assetManager, EditorState& editorState, const std::filesystem::path& assetRoot, const std::string& relativePath) {
    if (!editorState.getPause()) {
        return {SceneRequestStatus::Rejected, "Engine must be paused in order to open scene"};
    }

    if (editorState.getPendingAction() != PendingSceneAction::None) {
        return {SceneRequestStatus::Rejected, "Another scene action is already pending"};
    }

    try {
    
        std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);
        resolveScenePath(assetRoot, relativePath);

        if (editorState.isDirty()){
            editorState.setPendingOpen(normalizedScenePath);
            return {SceneRequestStatus::AwaitingConfirmation, "Scene is dirty and is awaiting confirmation before opening new scene"};
        }

        SceneFileResult openSceneResult = openScene(sceneManager, sceneSerializer, assetManager, editorState, assetRoot, relativePath);

        if (openSceneResult.success){
            return {SceneRequestStatus::Opened, openSceneResult.message};
        } else {
            return {SceneRequestStatus::Rejected, openSceneResult.message};
        }

    } catch (const std::exception& e) {
        return {SceneRequestStatus::Rejected, std::string("Opening scene unsucessful : ") + e.what()};  
    }

}

SceneRequestResult requestClose(EditorState& editorState) {
    if (editorState.getPendingAction() != PendingSceneAction::None) {
        return {SceneRequestStatus::Rejected, "Another scene action is already pending"};
    }

    if (editorState.isDirty()) {
        editorState.pause();
        editorState.setPendingClose();
        return {SceneRequestStatus::AwaitingConfirmation, "You have unsaved changes. Save before closing?"};
    }

    return {SceneRequestStatus::ReadyToClose, "Scene can be closed"};
}