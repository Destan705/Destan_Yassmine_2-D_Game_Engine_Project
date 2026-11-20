#pragma once

#include "imgui.h"
#include "logger.h"

// One editor-only look-and-feel.  Apply this BEFORE capturing the DPI baseline;
// ScaleAllSizes() then works from the same unscaled design on every monitor.
namespace EditorTheme {

void apply(ImGuiStyle& style);

ImVec4 accentText();
ImVec4 mutedText();
ImVec4 warningText();
ImVec4 playbackText(bool paused);
ImVec4 logText(logger_level severity);

// Three button colors are pushed.  Every push MUST be paired with popButton().
void pushPlaybackButton(bool paused);
void pushPrimaryButton();
void popButton();

} // namespace EditorTheme
