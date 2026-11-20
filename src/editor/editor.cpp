#include "editor.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "platform.h"
#include "logger.h"

Editor::Editor(SDL_Window* window, SDL_Renderer* renderer) {

    sdlRenderer = renderer;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Initialize ImGui backends.
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    // Subscribe to engine logs.
    logListenerId = subscribeToLog([this](logger_level level, const std::string& message) {
        logBuffer.add(level, message);
    });
}

Editor::~Editor() {

    unsubscribeFromLog(logListenerId);

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}


// Start a new ImGui frame.

void Editor::beginFrame() {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}


// Render the editor windows.

void Editor::render() {
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdlRenderer);
}


// Draw all editor panels

SceneFileCommand Editor::drawPanels(EditorState& editorState, Scene& scene, AssetManager& assetManager) {

    hierarchyPanel.draw(editorState, &scene);
    inspectorPanel.draw(editorState, scene, assetManager);
    consolePanel.draw(logBuffer, editorState);

    // Collider overlay controls.
    if (ImGui::Begin("Overlay")) {
        bool show = editorState.getShowColliders();
        ImGui::Checkbox("Show colliders", &show);
        editorState.setShowColliders(show);
    }

    ImGui::End();

    // Draw the Scene File panel and return its command.
    return sceneFilePanel.draw(editorState);
}

void Editor::drawStatsPanel(const StatsSnapshot& stats){
    statsPanel.draw(stats);
}


// Scene File panel helpers.

void Editor::setSceneFileFeedback(const std::string& message) {
    sceneFilePanel.setFeedbackMessage(message);
}

bool Editor::isStandaloneSaveRequested() const {
    return sceneFilePanel.isStandaloneSaveRequested();
}