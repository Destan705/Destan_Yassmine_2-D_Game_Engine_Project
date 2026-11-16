#include "scene_file_panel.h"
#include "imgui.h"
#include "editor_state.h"
#include <string>

SceneFileCommand SceneFilePanel::draw(const EditorState& editorState) {
    SceneFileCommand sceneFileCommand;

    if (lastSceneGeneration != editorState.getSceneGeneration()){
        pathBuffer[0] = '\0';
        savePathBuffer[0] = '\0';
        standaloneSaveRequested = false;
        lastSceneGeneration = editorState.getSceneGeneration();
    }

    if (ImGui::Begin("Scene File")) {

        auto scenePath = editorState.getCurrentScenePath();

        // Current scene information.
        ImGui::Text("Current Scene");

        if (scenePath.has_value()) {
            ImGui::TextUnformatted(scenePath->generic_string().c_str());
        } else {
            ImGui::Text("Untitled");
        }

        if (editorState.isDirty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "Scene has unsaved changes");
        } else {
            ImGui::TextDisabled("No unsaved changes");
        }

        ImGui::Spacing();

        ImGui::BeginDisabled(!editorState.getPause() || editorState.getPendingAction() != PendingSceneAction::None || standaloneSaveRequested);

        // Save the currently active scene.
        if (ImGui::Button("Save Current Scene")) {
            if (scenePath.has_value()) {
                sceneFileCommand.type = SceneFileCommandType::Save;
            } else {
                standaloneSaveRequested = true;
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Open or save to a different scene file.
        ImGui::Text("Scene File Path");

        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputTextWithHint("##SceneFilePath", "scenes/example.json", pathBuffer, sizeof(pathBuffer));

        if (ImGui::Button("Open")) {
            sceneFileCommand.type = SceneFileCommandType::Open;
            sceneFileCommand.path = std::string(pathBuffer);
        }

        ImGui::SameLine();

        if (ImGui::Button("Save As")) {
            sceneFileCommand.type = SceneFileCommandType::SaveAs;
            sceneFileCommand.path = std::string(pathBuffer);
        }

        ImGui::EndDisabled();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Operation feedback.
        if (!editorState.getPause()) {
            ImGui::TextDisabled("Pause the engine to manage scene files.");
        }

        ImGui::TextWrapped("%s", feedbackMessage.c_str());
    }

    drawConfirmationPopup(editorState, sceneFileCommand);
    drawSavePathPopup(editorState, sceneFileCommand);

    ImGui::End();

    return sceneFileCommand;
}

void SceneFilePanel::setFeedbackMessage(const std::string& message){
    feedbackMessage = message;
}

void SceneFilePanel::drawConfirmationPopup(const EditorState& editorState, SceneFileCommand& command){
    if (editorState.getPendingAction() == PendingSceneAction::None || editorState.isAwaitingSavePath()){
        return;
    }
    if (!ImGui::IsPopupOpen("Unsaved Changes")){
        ImGui::OpenPopup("Unsaved Changes");
    }
    if (ImGui::BeginPopupModal("Unsaved Changes")){
        if (editorState.getPendingAction() == PendingSceneAction::Open){
            ImGui::Text("You have unsaved changes. Would you like to save before opening another scene?");
        } else if (editorState.getPendingAction() == PendingSceneAction::Close)
        {
            ImGui::Text("You have unsaved changes. Would you like to save before closing the application?");
        }
        
        if (ImGui::Button("Save")){
            command.type = SceneFileCommandType::ConfirmSave;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard")){
            command.type = SceneFileCommandType::ConfirmDiscard;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")){
            command.type = SceneFileCommandType::ConfirmCancel;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SceneFilePanel::drawSavePathPopup(const EditorState& editorState, SceneFileCommand& command){
    if (!standaloneSaveRequested && !editorState.isAwaitingSavePath()){
        return;
    }
    if (!ImGui::IsPopupOpen("Save Scene")){
        savePathBuffer[0] = '\0';
        ImGui::OpenPopup("Save Scene");
    }
    if (ImGui::BeginPopupModal("Save Scene")){
        
        ImGui::Text("Please provide a save location.");

        ImGui::InputText("Path:", savePathBuffer, sizeof(savePathBuffer));

        if (ImGui::Button("Save")){
            command.type = editorState.isAwaitingSavePath() ? SceneFileCommandType::ProvideSavePath : SceneFileCommandType::SaveAs;
            command.path = std::string(savePathBuffer);
            standaloneSaveRequested = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")){
            command.type = editorState.isAwaitingSavePath() ? SceneFileCommandType::ConfirmCancel : SceneFileCommandType::None;
            standaloneSaveRequested = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool SceneFilePanel::isStandaloneSaveRequested() const {
    return standaloneSaveRequested;
}