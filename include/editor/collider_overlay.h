#pragma once
#include "renderer.h"
#include "scene.h"
#include "editor_state.h"

// Draws an outline around every enabled collider (editor build only).
// Call after the scene is drawn and before ImGui, so outlines sit on top of
// sprites but under the editor panels.
//
// Skipped: entities with no Transform or no Collider, disabled colliders,
// and boxes with non-finite values.
//
// Colours:
//   green  {0, 255, 0, 255}   normal collider
//   red    {255, 0, 0, 255}   overlapping another collider this frame
//   yellow {255, 255, 0, 255} selected entity (wins over red)
void drawColliderOverlay(Renderer& renderer, Scene& scene, const EditorState& editorState);