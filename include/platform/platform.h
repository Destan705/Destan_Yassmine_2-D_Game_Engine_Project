#pragma once

#include "rendertypes.h"
#include "scene.h"
#include <filesystem>
#ifdef ENGINE_EDITOR
#include "editor_state.h"
#include "scene_file_panel.h"

#endif

class Renderer;
class AssetManager;

bool initializePlatform();
void shutdownPlatform();
void renderPlatform(Scene& scene);
textureHandle loadPlatformTexture(const char* filePath);
std::filesystem::path getAssetRootPath();
AssetManager& getPlatformAssetManager();
void beginEditorFrame();
#ifdef ENGINE_EDITOR
SceneFileCommand drawEditorPanels(Scene& scene, EditorState& editorState);
void setSceneFileFeedback(const std::string& message);
bool isStandaloneSaveRequested();
#endif