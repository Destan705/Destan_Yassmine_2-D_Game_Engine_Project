#include "scene_file_panel.h"
#include "imgui.h"
#include "editor_state.h"
#include <string>

SceneFileCommand SceneFilePanel::draw(const EditorState& editorState) {
    SceneFileCommand sceneFileCommand;

    if (ImGui::Begin("SceneFile")) {

        auto scenePath = editorState.getCurrentScenePath();

        if (scenePath.has_value()) {
            ImGui::TextUnformatted(scenePath.value().generic_string().c_str());
        } else {
            ImGui::Text("Untitled");
        }

        editorState.isDirty() ? ImGui::Text("Scene has unsaved changes") : ImGui::Text("No unsaved changes");
        
        ImGui::BeginDisabled(!editorState.getPause() || editorState.getPendingAction() != PendingSceneAction::None || standaloneSaveRequested);
        
        ImGui::InputText("Path:", pathBuffer, sizeof(pathBuffer));

        if (ImGui::Button("Open")){
            sceneFileCommand.type = SceneFileCommandType::Open;
            sceneFileCommand.path = std::string(pathBuffer);
        }
        ImGui::SameLine();
        if (ImGui::Button("Save")){
            if (scenePath.has_value()) {
                sceneFileCommand.type = SceneFileCommandType::Save;
            } else {
                standaloneSaveRequested = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save As")){
            sceneFileCommand.type = SceneFileCommandType::SaveAs;
            sceneFileCommand.path = std::string(pathBuffer);
        }

        ImGui::EndDisabled();

        ImGui::TextUnformatted(feedbackMessage.c_str());
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