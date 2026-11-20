#include "scene_file_panel.h"
#include "editor_layout.h"
#include "imgui.h"
#include "editor_theme.h"
#include "editor_state.h"
#include <string>
#include <algorithm>
#include <cfloat>
#include <cstdio>


// PHASE4_WIDGET_POLISH_V1: shared modal layout helpers.
namespace {
void beginCenteredDialogLayout() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float dpi = std::clamp(ImGui::GetStyle().FontScaleDpi, 0.75f, 4.0f);
    const float width = std::min(540.0f * dpi, viewport->WorkSize.x * 0.88f);
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Appearing);
}

float dialogButtonWidth(const char* label) {
    const float dpi = std::clamp(ImGui::GetStyle().FontScaleDpi, 0.75f, 4.0f);
    return std::max(82.0f * dpi, ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2.0f);
}

void alignDialogActions(float combinedWidth) {
    const float remaining = ImGui::GetContentRegionAvail().x;
    if (remaining > combinedWidth) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + remaining - combinedWidth);
    }
}
}

SceneFileCommand SceneFilePanel::draw(const EditorState& editorState) {
    SceneFileCommand command{};
    if (lastSceneGeneration != editorState.getSceneGeneration()) {
        pathBuffer[0] = '\0';
        savePathBuffer[0] = '\0';
        standaloneSaveRequested = false;
        openDialogRequested = false;
        lastSceneGeneration = editorState.getSceneGeneration();
    }

    drawOpenPathPopup(command);
    drawConfirmationPopup(editorState, command);
    drawSavePathPopup(editorState, command);
    return command;
}

void SceneFilePanel::setFeedbackMessage(const std::string& message){
    feedbackMessage = message;
}

void SceneFilePanel::drawConfirmationPopup(const EditorState& editorState, SceneFileCommand& command) {
    if (editorState.getPendingAction() == PendingSceneAction::None || editorState.isAwaitingSavePath()) {
        return;
    }
    if (!ImGui::IsPopupOpen("Unsaved Changes")) {
        ImGui::OpenPopup("Unsaved Changes");
    }
    beginCenteredDialogLayout();
    if (ImGui::BeginPopupModal("Unsaved Changes", nullptr, ImGuiWindowFlags_NoResize)) {
        ImGui::TextColored(EditorTheme::warningText(), "UNSAVED CHANGES");
        ImGui::Spacing();
        if (editorState.getPendingAction() == PendingSceneAction::Open) {
            ImGui::TextWrapped("Save your changes before opening another scene?");
        } else {
            ImGui::TextWrapped("Save your changes before closing the editor?");
        }
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        const float saveWidth = dialogButtonWidth("Save");
        const float discardWidth = dialogButtonWidth("Discard");
        const float cancelWidth = dialogButtonWidth("Cancel");
        alignDialogActions(saveWidth + discardWidth + cancelWidth +
                           ImGui::GetStyle().ItemSpacing.x * 2.0f);
        EditorTheme::pushPrimaryButton();
        if (ImGui::Button("Save", ImVec2(saveWidth, 0.0f))) {
            command.type = SceneFileCommandType::ConfirmSave;
            ImGui::CloseCurrentPopup();
        }
        EditorTheme::popButton();
        ImGui::SameLine();
        if (ImGui::Button("Discard", ImVec2(discardWidth, 0.0f))) {
            command.type = SceneFileCommandType::ConfirmDiscard;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(cancelWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            command.type = SceneFileCommandType::ConfirmCancel;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SceneFilePanel::drawSavePathPopup(const EditorState& editorState, SceneFileCommand& command) {
    if (!standaloneSaveRequested && !editorState.isAwaitingSavePath()) { return; }
    if (!ImGui::IsPopupOpen("Save Scene")) {
        savePathBuffer[0] = '\0';
        if (const auto current = editorState.getCurrentScenePath()) {
            std::snprintf(savePathBuffer, sizeof(savePathBuffer), "%s", current->generic_string().c_str());
        }
        ImGui::OpenPopup("Save Scene");
    }
    beginCenteredDialogLayout();
    if (ImGui::BeginPopupModal("Save Scene", nullptr, ImGuiWindowFlags_NoResize)) {
        ImGui::TextColored(EditorTheme::accentText(), "SAVE SCENE AS");
        ImGui::Spacing();
        ImGui::TextWrapped("Choose a location for your scene file.");
        ImGui::Spacing();
        ImGui::TextDisabled("Destination path");
        ImGui::SetNextItemWidth(-FLT_MIN);
        const bool submitted = ImGui::InputTextWithHint(
            "##SaveScenePath", "scenes/new-scene.json", savePathBuffer, sizeof(savePathBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::TextDisabled("Scene files use the .json extension.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float actionWidth = dialogButtonWidth("Save");
        const float cancelWidth = dialogButtonWidth("Cancel");
        alignDialogActions(actionWidth + cancelWidth + ImGui::GetStyle().ItemSpacing.x);
        ImGui::BeginDisabled(savePathBuffer[0] == '\0');
        EditorTheme::pushPrimaryButton();
        const bool clicked = ImGui::Button("Save", ImVec2(actionWidth, 0.0f));
        EditorTheme::popButton();
        ImGui::EndDisabled();
        if ((clicked || submitted) && savePathBuffer[0] != '\0') {
            command.type = editorState.isAwaitingSavePath()
                ? SceneFileCommandType::ProvideSavePath
                : SceneFileCommandType::SaveAs;
            command.path = savePathBuffer;
            standaloneSaveRequested = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(cancelWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            command.type = editorState.isAwaitingSavePath()
                ? SceneFileCommandType::ConfirmCancel
                : SceneFileCommandType::None;
            standaloneSaveRequested = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool SceneFilePanel::isStandaloneSaveRequested() const {
    return standaloneSaveRequested;
}

void SceneFilePanel::requestOpenDialog() {
    openDialogRequested = true;
}

void SceneFilePanel::requestSaveAsDialog() {
    standaloneSaveRequested = true;
}

void SceneFilePanel::drawOpenPathPopup(SceneFileCommand& command) {
    if (!openDialogRequested) { return; }
    if (!ImGui::IsPopupOpen("Open Scene")) {
        pathBuffer[0] = '\0';
        ImGui::OpenPopup("Open Scene");
    }
    beginCenteredDialogLayout();
    if (ImGui::BeginPopupModal("Open Scene", nullptr, ImGuiWindowFlags_NoResize)) {
        ImGui::TextColored(EditorTheme::accentText(), "OPEN SCENE");
        ImGui::Spacing();
        ImGui::TextWrapped("Enter the path of a scene inside your assets folder.");
        ImGui::Spacing();
        ImGui::TextDisabled("Scene file path");
        ImGui::SetNextItemWidth(-FLT_MIN);
        const bool submitted = ImGui::InputTextWithHint(
            "##OpenScenePath", "scenes/phase3-demo.json", pathBuffer, sizeof(pathBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::TextDisabled("Example: scenes/phase3-demo.json");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const float actionWidth = dialogButtonWidth("Open");
        const float cancelWidth = dialogButtonWidth("Cancel");
        alignDialogActions(actionWidth + cancelWidth + ImGui::GetStyle().ItemSpacing.x);
        ImGui::BeginDisabled(pathBuffer[0] == '\0');
        EditorTheme::pushPrimaryButton();
        const bool clicked = ImGui::Button("Open", ImVec2(actionWidth, 0.0f));
        EditorTheme::popButton();
        ImGui::EndDisabled();
        if ((clicked || submitted) && pathBuffer[0] != '\0') {
            command.type = SceneFileCommandType::Open;
            command.path = pathBuffer;
            openDialogRequested = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(cancelWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            openDialogRequested = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

const std::string& SceneFilePanel::getFeedbackMessage() const {
    return feedbackMessage;
}

bool SceneFilePanel::isModalInteractionActive() const {
    return openDialogRequested || standaloneSaveRequested;
}
