#include "console_panel.h"
#include "logger.h"
#include "imgui.h"
#include <string>


void ConsolePanel::draw(LogBuffer& logBuffer, EditorState& editorState){
    ImGui::SetNextWindowPos(ImVec2(20, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Console");

    ImGui::Checkbox("Info", &showInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Warning", &showWarning);
    ImGui::SameLine();
    ImGui::Checkbox("Error", &showError);

    ImGui::InputText("Search", searchBuffer, sizeof(searchBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        logBuffer.clear();
    }
    
    ImGui::BeginChild("LogScroll", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    
    for(const auto& entry : logBuffer.getEntries()){
        
        if (entry.severity == logger_level::Info && !showInfo) continue;
        if (entry.severity == logger_level::Warning && !showWarning) continue;
        if (entry.severity == logger_level::Error && !showError) continue;
        if (entry.message.find(searchBuffer) == std::string::npos) continue;

        ImGui::TextUnformatted(entry.message.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()){
    ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();

    if (ImGui::InputText("Command", commandBuffer, sizeof(commandBuffer),ImGuiInputTextFlags_EnterReturnsTrue)) {

        std::string command = commandBuffer;
        if (!command.empty()) { 
            std::string name = command;
            std::string args;

            size_t space = command.find(' ');
            if (space != std::string::npos) {
                name = command.substr(0, space);   // before the space
                args = command.substr(space + 1);  // after the space
            }

            bool known = name == "help" || name == "clear" ||
                name == "pause" || name == "resume";

            if (!known) {
                logBuffer.add(logger_level::Error, "Unknown command: " + name);
            } else if (!args.empty()) {
            logBuffer.add(logger_level::Error, name + " takes no arguments");
            }
            else if (name == "help") {
            logBuffer.add(logger_level::Info, "Commands: help, clear, pause, resume");
            } 
            else if (name == "clear") {
            logBuffer.clear();
            } 
            else if (name == "pause") {
                editorState.pause();
            } 
            else if (name == "resume") {
                editorState.resume();
            } 

        }

        commandBuffer[0] = '\0';

         
    }

    ImGui::End();
}
