#pragma once
#include <SDL3/SDL.h>
#include "inspector_panel.h"
#include "hierarchy_panel.h"

// Owns Dear ImGui for the editor build.
// Created by the platform after the Renderer exists, destroyed before it.
// ImGui headers are only included in editor.cpp, never here.
class Editor {
  public:
    // Creates the ImGui context and starts the SDL3 input backend
    // and the SDL_Renderer drawing backend.
    Editor(SDL_Window* window, SDL_Renderer* renderer);

    // Shuts the backends down and destroys the ImGui context,
    // in the reverse order of the constructor.
    ~Editor();

    // No copying: two copies would both shut ImGui down,
    // and the second shutdown would crash.
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    // Starts a new ImGui frame. Call once per frame, after input is processed
    // and before any editor windows are built.
    void beginFrame();

    // Finishes the ImGui frame and draws the editor windows on top of the scene.
    // Call after the scene is drawn and before Present().
    void render();

    void drawPanels(EditorState& editorState,Scene& scene, AssetManager& assetManager);

    private:

    SDL_Renderer* sdlRenderer = nullptr;

    InspectorPanel inspectorPanel;

    HierarchyPanel hierarchyPanel;

    

};