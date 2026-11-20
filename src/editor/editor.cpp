#include "editor.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "platform.h"
#include "logger.h"
#include "editor_dpi_scale.h"
#include "editor_theme.h"
#include "editor_icon_button.h"
#include "editor_toolbar_policy.h"
#include "input.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

Editor::Editor(SDL_Window* window, SDL_Renderer* renderer) {

    sdlRenderer = renderer;
    editorWindow = window;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Record original dimensions once. Rebuild from them on every DPI change
    // instead of repeatedly scaling already-scaled styles.
    EditorTheme::apply(ImGui::GetStyle());
    unscaledStyle = ImGui::GetStyle();
    updateDpiScale();

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

    // SDL texture must die while SDL_Renderer is still alive.
    if (sceneTexture != nullptr) {
        SDL_DestroyTexture(sceneTexture);
        sceneTexture = nullptr;
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}


// Start a new ImGui frame.

void Editor::beginFrame() {
    // Monitor transitions and Windows scale changes are picked up before UI draw.
    updateDpiScale();
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
    layoutRegions = EditorLayout::calculate(*ImGui::GetMainViewport());
    SceneFileCommand fileCommand{};

    EditorLayout::place(layoutRegions.toolbar);
    if (ImGui::Begin("##EditorToolbar", nullptr,
                     EditorLayout::barFlags | ImGuiWindowFlags_MenuBar)) {
        const bool dialogOpen = sceneFilePanel.isModalInteractionActive();
        const bool canEditFiles = EditorToolbarPolicy::canUseSceneFiles(editorState, dialogOpen);
        const bool canPlay = EditorToolbarPolicy::canTogglePlayback(editorState, dialogOpen);

        // Both the top menu and toolbar use the existing scene-file command flow.
        const auto openSceneDialog = [&]() { sceneFilePanel.requestOpenDialog(); };
        const auto saveScene = [&]() {
            if (editorState.getCurrentScenePath().has_value()) {
                fileCommand.type = SceneFileCommandType::Save;
            } else {
                sceneFilePanel.requestSaveAsDialog();
            }
        };
        const auto saveSceneAs = [&]() { sceneFilePanel.requestSaveAsDialog(); };
        const auto togglePlay = [&]() {
            if (editorState.getPause()) {
                editorState.resume();
            } else {
                editorState.pause();
            }
        };

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                ImGui::BeginDisabled(!canEditFiles);
                if (ImGui::MenuItem("Open Scene...")) { openSceneDialog(); }
                if (ImGui::MenuItem("Save Scene")) { saveScene(); }
                if (ImGui::MenuItem("Save Scene As...")) { saveSceneAs(); }
                ImGui::EndDisabled();
                ImGui::Separator();
                const auto currentPath = editorState.getCurrentScenePath();
                if (currentPath.has_value()) {
                    ImGui::TextDisabled("Current: %s", currentPath->generic_string().c_str());
                } else {
                    ImGui::TextDisabled("Current: Untitled");
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                bool showColliders = editorState.getShowColliders();
                if (ImGui::MenuItem("Show Collider Bounds", nullptr, &showColliders)) {
                    editorState.setShowColliders(showColliders);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                ImGui::TextDisabled("F1: Pause / Resume");
                ImGui::TextDisabled("Click Scene View to focus gameplay input");
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Icon-only actions fit smaller windows and scale with ImGui font DPI.
        ImGui::TextColored(EditorTheme::accentText(), "SCENE");
        ImGui::SameLine();

        ImGui::BeginDisabled(!canPlay);
        EditorTheme::pushPlaybackButton(editorState.getPause());
        const bool playbackClicked = EditorIconButton::draw(
            "##PlayPause", editorState.getPause() ? EditorIconButton::Icon::Play
                                                 : EditorIconButton::Icon::Pause,
            editorState.getPause() ? "Play / Resume (F1)" : "Pause (F1)");
        EditorTheme::popButton();
        ImGui::EndDisabled();
        if (playbackClicked) { togglePlay(); }

        ImGui::SameLine();
        ImGui::BeginDisabled(!canEditFiles);
        const bool openClicked = EditorIconButton::draw(
            "##OpenScene", EditorIconButton::Icon::Open, "Open Scene...");
        ImGui::SameLine();
        const bool saveClicked = EditorIconButton::draw(
            "##SaveScene", EditorIconButton::Icon::Save, "Save Scene");
        ImGui::SameLine();
        const bool saveAsClicked = EditorIconButton::draw(
            "##SaveAsScene", EditorIconButton::Icon::SaveAs, "Save Scene As...");
        ImGui::EndDisabled();
        if (openClicked) { openSceneDialog(); }
        if (saveClicked) { saveScene(); }
        if (saveAsClicked) { saveSceneAs(); }

        ImGui::SameLine();
        const bool showColliders = editorState.getShowColliders();
        if (EditorIconButton::draw(
                "##ColliderOverlay", EditorIconButton::Icon::Colliders,
                showColliders ? "Hide Collider Bounds" : "Show Collider Bounds",
                showColliders)) {
            editorState.setShowColliders(!showColliders);
        }

        // These modals share the toolbar's stable ImGui ID context.
        // The engine still applies commands at the safe end-of-frame boundary.
        SceneFileCommand modalCommand = sceneFilePanel.draw(editorState);
        if (modalCommand.type != SceneFileCommandType::None) {
            fileCommand = std::move(modalCommand);
        }
    }
    ImGui::End();

    EditorLayout::place(layoutRegions.hierarchy);
    hierarchyPanel.draw(editorState, &scene);

    EditorLayout::place(layoutRegions.inspector);
    inspectorPanel.draw(editorState, scene, assetManager);

    EditorLayout::place(layoutRegions.console);
    consolePanel.draw(logBuffer, editorState);

    // Step 2's render target and viewport coordinate handling are unchanged.
    EditorLayout::place(layoutRegions.scene);
    sceneViewportGeometry = {};
    sceneImageHovered = false;
    bool sceneFocused = false;
    if (ImGui::Begin("Scene View", nullptr,
                     EditorLayout::panelFlags | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse)) {
        const ImVec2 available = ImGui::GetContentRegionAvail();
        const float displayWidth = std::max(1.0f, available.x);
        const float displayHeight = std::max(1.0f, available.y);

        const ImVec2 framebufferScale = ImGui::GetIO().DisplayFramebufferScale;
        const float scaleX = framebufferScale.x > 0.0f ? framebufferScale.x : 1.0f;
        const float scaleY = framebufferScale.y > 0.0f ? framebufferScale.y : 1.0f;
        const int pixelWidth = std::clamp(
            static_cast<int>(std::ceil(displayWidth * scaleX)), 1, 8192);
        const int pixelHeight = std::clamp(
            static_cast<int>(std::ceil(displayHeight * scaleY)), 1, 8192);

        if (ensureSceneTexture(pixelWidth, pixelHeight)) {
            const ImTextureID id = static_cast<ImTextureID>(
                reinterpret_cast<std::uintptr_t>(sceneTexture));
            ImGui::Image(ImTextureRef(id), ImVec2(displayWidth, displayHeight));

            const ImVec2 imageOrigin = ImGui::GetItemRectMin();
            const ImVec2 imageSize = ImGui::GetItemRectSize();
            sceneViewportGeometry = {imageOrigin.x, imageOrigin.y,
                                     imageSize.x, imageSize.y,
                                     sceneTextureWidth, sceneTextureHeight};
            sceneImageHovered = ImGui::IsItemHovered();
            if (sceneImageHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ImGui::SetWindowFocus();
            }
            sceneFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        } else {
            ImGui::TextWrapped("Scene viewport unavailable: %s", sceneTextureError.c_str());
        }
    }
    ImGui::End();

    setGameplayViewportActive(sceneFocused);

    EditorLayout::place(layoutRegions.status);
    if (ImGui::Begin("##EditorStatus", nullptr, EditorLayout::barFlags)) {
        ImGui::Text("Scene: %s", scene.getSceneName().c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("|");
        ImGui::SameLine();
        ImGui::TextColored(EditorTheme::playbackText(editorState.getPause()),
                            "%s", editorState.getPause() ? "Paused" : "Running");
        if (editorState.isDirty()) {
            ImGui::SameLine();
            ImGui::TextColored(EditorTheme::warningText(), "| Unsaved changes");
        }
        const std::string& feedback = sceneFilePanel.getFeedbackMessage();
        if (!feedback.empty()) {
            ImGui::SameLine();
            ImGui::TextDisabled("|");
            ImGui::SameLine();
            ImGui::TextUnformatted(feedback.c_str());
            if (ImGui::IsItemHovered()) { ImGui::SetTooltip("%s", feedback.c_str()); }
        }
    }
    ImGui::End();

    return fileCommand;
}

void Editor::drawStatsPanel(const StatsSnapshot& stats) {
    EditorLayout::place(layoutRegions.statistics);
    statsPanel.draw(stats);
}


// Scene File panel helpers.

void Editor::setSceneFileFeedback(const std::string& message) {
    sceneFilePanel.setFeedbackMessage(message);
}

bool Editor::isStandaloneSaveRequested() const {
    return sceneFilePanel.isModalInteractionActive();
}

bool Editor::ensureSceneTexture(int width, int height) {
    if (sceneTexture != nullptr && sceneTextureWidth == width && sceneTextureHeight == height) {
        return true;
    }

    SDL_Texture* newTexture = SDL_CreateTexture(
        sdlRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (newTexture == nullptr) {
        sceneTextureError = SDL_GetError();
        return sceneTexture != nullptr; // Keep old frame if resizing fails.
    }

    SDL_SetTextureScaleMode(newTexture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(newTexture, SDL_BLENDMODE_NONE);
    if (sceneTexture != nullptr) {
        SDL_DestroyTexture(sceneTexture);
    }
    sceneTexture = newTexture;
    sceneTextureWidth = width;
    sceneTextureHeight = height;
    sceneTextureError.clear();
    return true;
}

SDL_Texture* Editor::getSceneTexture() const {
    return sceneTexture;
}

std::optional<std::pair<float, float>> Editor::getSceneMousePosition() const {
    if (!sceneImageHovered) {
        return std::nullopt;
    }
    const ImVec2 mouse = ImGui::GetIO().MousePos;
    return sceneViewportGeometry.screenToScene(mouse.x, mouse.y);
}


// Dynamic DPI adjustment is editor-only. Scene/world dimensions are unchanged.
void Editor::updateDpiScale() {
    const float detected = SDL_GetWindowDisplayScale(editorWindow);
    const float next = EditorDpi::normalizedScale(
        detected, appliedDpiScale > 0.0f ? appliedDpiScale : 1.0f);
    if (!EditorDpi::scaleChanged(appliedDpiScale, next)) {
        return;
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style = unscaledStyle;
    style.ScaleAllSizes(next);
    // ImGui 1.92 supports DPI-aware font rasterization at runtime.
    style.FontScaleDpi = unscaledStyle.FontScaleDpi * next;
    appliedDpiScale = next;
}
