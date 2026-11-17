#include "console_panel.h"
#include "imgui.h"

void ConsolePanel::draw(LogBuffer& logBuffer){
    ImGui::Begin("Console");
    ImGui::SetNextWindowPos(ImVec2(20, 400), ImGuiCond_FirstUseEver);
    for(auto& entry:logBuffer.getEntries()){
        ImGui::TextUnformatted(entry.message.c_str());
    }
    ImGui::End();
}