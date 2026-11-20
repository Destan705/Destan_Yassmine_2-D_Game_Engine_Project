#include "console_panel.h"
#include "console_commands.h"
#include "imgui.h"

void ConsolePanel::draw(LogBuffer& logBuffer, EditorState& editorState) {
    ImGui::SetNextWindowPos(ImVec2(20, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Console")) {
        ImGui::Checkbox("Info", &showInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warning", &showWarning);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &showError);

        ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Clear")) logBuffer.clear();

        ImGui::BeginChild("LogScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
        for (const auto& entry : logBuffer.getEntries()) {
            if (consoleLogMatches(entry, showInfo, showWarning, showError, searchBuffer)) {
                ImGui::TextUnformatted(entry.message.c_str());
            }
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();

        if (ImGui::InputText("Command", commandBuffer, sizeof(commandBuffer),
                             ImGuiInputTextFlags_EnterReturnsTrue)) {
            executeConsoleCommand(commandBuffer, logBuffer, editorState);
            commandBuffer[0] = '\0';
        }
    }
    ImGui::End();
}
