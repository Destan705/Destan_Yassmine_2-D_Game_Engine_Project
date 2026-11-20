// PHASE4_WIDGET_POLISH_V1
#include "console_panel.h"
#include "editor_layout.h"
#include "console_commands.h"
#include "imgui.h"
#include <algorithm>
#include <cfloat>
#include "editor_theme.h"

void ConsolePanel::draw(LogBuffer& logBuffer, EditorState& editorState) {
    if (ImGui::Begin("Console")) {
        // Filters stay grouped instead of being spread across the entire row.
        ImGui::TextDisabled("SHOW");
        ImGui::SameLine();
        ImGui::Checkbox("Info", &showInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warning", &showWarning);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &showError);
        ImGui::Separator();

        const float dpi = std::clamp(ImGui::GetStyle().FontScaleDpi, 0.75f, 4.0f);
        const float clearWidth = std::max(64.0f * dpi, ImGui::CalcTextSize("Clear").x +
                                   2.0f * ImGui::GetStyle().FramePadding.x);
        const float available = ImGui::GetContentRegionAvail().x;
        const bool compact = available < clearWidth + 120.0f * dpi;
        const float searchWidth = compact ? -FLT_MIN :
            std::max(40.0f, available - clearWidth - ImGui::GetStyle().ItemSpacing.x);
        ImGui::SetNextItemWidth(searchWidth);
        ImGui::InputTextWithHint("##ConsoleSearch", "Search log messages...",
                                  searchBuffer, sizeof(searchBuffer));
        if (!compact) { ImGui::SameLine(); }
        if (ImGui::Button("Clear", ImVec2(clearWidth, 0.0f))) {
            logBuffer.clear();
        }

        ImGui::Spacing();
        // Reserve a command line even when the Console panel is short.
        const float reserved = ImGui::GetFrameHeightWithSpacing() +
                               ImGui::GetStyle().ItemSpacing.y * 3.0f;
        const float logHeight = std::max(32.0f, ImGui::GetContentRegionAvail().y - reserved);
        if (ImGui::BeginChild("##ConsoleLogScroll", ImVec2(0.0f, logHeight),
                              ImGuiChildFlags_Borders)) {
            const bool wasAtBottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 2.0f;
            int visibleCount = 0;
            for (const auto& entry : logBuffer.getEntries()) {
                if (!consoleLogMatches(entry, showInfo, showWarning, showError, searchBuffer)) {
                    continue;
                }
                ++visibleCount;
                const char* level = entry.severity == logger_level::Error ? "ERROR"
                    : entry.severity == logger_level::Warning ? "WARN" : "INFO";
                ImGui::TextColored(EditorTheme::logText(entry.severity), "[%s]", level);
                ImGui::SameLine();
                ImGui::TextWrapped("%s", entry.message.c_str());
            }
            if (visibleCount == 0) {
                ImGui::Spacing();
                ImGui::TextDisabled("No log messages match the current filters.");
            }
            if (wasAtBottom && visibleCount > 0) { ImGui::SetScrollHereY(1.0f); }
        }
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::TextColored(EditorTheme::accentText(), ">");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputTextWithHint("##ConsoleCommand", "Enter a command and press Enter...",
                                     commandBuffer, sizeof(commandBuffer),
                                     ImGuiInputTextFlags_EnterReturnsTrue)) {
            executeConsoleCommand(commandBuffer, logBuffer, editorState);
            commandBuffer[0] = '\0';
        }
    }
    ImGui::End();
}
