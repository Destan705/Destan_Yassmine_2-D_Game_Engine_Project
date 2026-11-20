#pragma once

#include <SDL3/SDL.h>
#include "inspector_panel.h"
#include "hierarchy_panel.h"
#include "scene_file_panel.h"
#include "log_buffer.h"
#include "stats_panel.h"
#include "console_panel.h"
#include <cstdint>
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

    void setSceneFileFeedback(const std::string& message);
    bool isStandaloneSaveRequested() const;

private:
    SDL_Renderer* sdlRenderer = nullptr;

    InspectorPanel inspectorPanel;
    HierarchyPanel hierarchyPanel;
    SceneFilePanel sceneFilePanel;
    StatsPanel statsPanel;

    LogBuffer logBuffer;
    std::uint64_t logListenerId = 0;
    
    ConsolePanel consolePanel;

};