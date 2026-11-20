#pragma once

#include <SDL3/SDL.h>
#include "imgui.h" // ImGuiStyle retained as unscaled baseline for DPI changes
#include "editor_layout.h"
#include "scene_viewport_geometry.h"
#include "inspector_panel.h"
#include "hierarchy_panel.h"
#include "scene_file_panel.h"
#include "log_buffer.h"
#include "stats_panel.h"
#include "console_panel.h"
#include <cstdint>
#include <optional>
#include <utility>
#include <string>

// Owns Dear ImGui for the editor build.
// Created by the platform after the Renderer exists, destroyed before it.
class Editor {

public:
    Editor(SDL_Window* window, SDL_Renderer* renderer);

    ~Editor();

    // Prevent copying the Editor and its ImGui context.
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    // Starts a new ImGui frame.
    void beginFrame();

    // Renders the editor windows.
    void render();

    // Draws the editor panels and returns scene file commands.
    SceneFileCommand drawPanels(EditorState& editorState, Scene& scene, AssetManager& assetManager);

    void drawStatsPanel(const StatsSnapshot& stats);

    // Editor-owned offscreen Scene texture. Destroyed before SDL_Renderer.
    SDL_Texture* getSceneTexture() const;

    // Mouse pixels relative to the Scene image; nullopt outside its bounds.
    std::optional<std::pair<float, float>> getSceneMousePosition() const;

    void setSceneFileFeedback(const std::string& message);
    bool isStandaloneSaveRequested() const;

private:
    void updateDpiScale();
    SDL_Window* editorWindow = nullptr;
    ImGuiStyle unscaledStyle{};
    float appliedDpiScale = 0.0f;

    bool ensureSceneTexture(int width, int height);

    SceneViewportGeometry sceneViewportGeometry{};
    bool sceneImageHovered = false;
    SDL_Texture* sceneTexture = nullptr;
    int sceneTextureWidth = 0;
    int sceneTextureHeight = 0;
    std::string sceneTextureError;
    EditorLayoutRegions layoutRegions{};
    SDL_Renderer* sdlRenderer = nullptr;

    InspectorPanel inspectorPanel;
    HierarchyPanel hierarchyPanel;
    SceneFilePanel sceneFilePanel;
    StatsPanel statsPanel;

    LogBuffer logBuffer;
    std::uint64_t logListenerId = 0;
    
    ConsolePanel consolePanel;

};