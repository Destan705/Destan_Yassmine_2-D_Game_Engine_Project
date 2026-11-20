#!/usr/bin/env python3
"""One-time update for legacy source that cannot safely be replaced wholesale.

Run from the repository root AFTER extracting the ZIP into that root.
Updates two obsolete Sprite-required tests and integrates the input capture
state used by the Phase 4 regression suite. Refuses unexpected file content.
"""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parent


def update_test(path: str, old_name: str, new_body: str, new_name: str) -> None:
    file = ROOT / path
    content = file.read_text(encoding="utf-8")
    if new_name in content:
        print(f"Already updated: {path}: {new_name}")
        return
    pattern = rf"TEST_F\([^\n]*, {re.escape(old_name)}\)\s*\{{.*?\n\}}"
    revised, matches = re.subn(pattern, lambda _: new_body, content, count=1, flags=re.S)
    if matches != 1:
        raise RuntimeError(f"Cannot find original test {old_name} in {file}. No changes made.")
    file.write_text(revised, encoding="utf-8")
    print(f"Updated outdated test in {path}")


def update_input() -> None:
    file = ROOT / "src/platform/input.cpp"
    original = file.read_text(encoding="utf-8")
    if "InputCaptureState gameplayInput" in original:
        print("Input routing already updated")
        return
    if '#include "input.h"\n' not in original:
        raise RuntimeError("Could not locate input.h include")
    if "void initializeInput()" not in original or "SDL_Scancode toSDLScancode(Key key)" not in original:
        raise RuntimeError("Unexpected input.cpp; refusing unsafe rewrite")
    prefix = original[:original.index("void initializeInput()")]
    prefix = prefix.replace('#include "input.h"\n', '#include "input.h"\n#include "input_capture_state.h"\n', 1)
    # Only remove the old global bookkeeping; leave the complete Key-to-SDL
    # scancode translation and SDL event object untouched.
    old = ("const bool* keystate;\n"
           "std::array<bool, SDL_SCANCODE_COUNT> pressedThisFrame{};\n"
           "std::array<bool, SDL_SCANCODE_COUNT> releasedThisFrame{};\n"
           "std::array<bool, SDL_SCANCODE_COUNT> shortcutPressedThisFrame{};")
    if old not in prefix:
        raise RuntimeError("Unexpected input.cpp state declarations; refusing unsafe rewrite")
    prefix = prefix.replace(old, 'InputCaptureState gameplayInput;', 1)
    tail = r'''void initializeInput() {
    gameplayInput.reset();
}

// The gameplay gate is distinct from editor shortcut routing.
bool uiWantsKeyboard() {
#ifdef ENGINE_EDITOR
    return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard;
#else
    return false;
#endif
}

bool processPlatformEvents() {
    gameplayInput.beginFrame(uiWantsKeyboard());
    bool closeRequested = false;

    while (SDL_PollEvent(&event)) {
#ifdef ENGINE_EDITOR
        if (ImGui::GetCurrentContext() != nullptr) {
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
#endif
        gameplayInput.setCaptured(uiWantsKeyboard());

        // Always honor OS window close, even when typing into the UI.
        if (event.type == SDL_EVENT_QUIT ||
            event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            closeRequested = true;
        } else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
            gameplayInput.resetGameplay();
        } else if (event.type == SDL_EVENT_KEY_DOWN) {
            gameplayInput.keyDown(event.key.scancode, event.key.repeat);
        } else if (event.type == SDL_EVENT_KEY_UP) {
            gameplayInput.keyUp(event.key.scancode);
        }
    }
    gameplayInput.setCaptured(uiWantsKeyboard());
    return !closeRequested;
}

bool isKeyHeld(Key key) {
    return gameplayInput.isHeld(toSDLScancode(key));
}

bool wasKeyPressed(Key key) {
    return gameplayInput.wasPressed(toSDLScancode(key));
}

bool wasKeyReleased(Key key) {
    return gameplayInput.wasReleased(toSDLScancode(key));
}

bool wasShortcutPressed(Key key) {
    return gameplayInput.shortcutPressed(toSDLScancode(key));
}
'''
    file.write_text(prefix + tail, encoding="utf-8")
    print("Updated gameplay input capture/held/edge routing")


def update_inspector_label() -> None:
    file = ROOT / "src/editor/inspector_panel.cpp"
    content = file.read_text(encoding="utf-8")
    old = 'ImGui::Text("Entity %llu", static_cast<unsigned long long>(id));'
    new = (old + '\n                ImGui::Text("Name: %s", selected->getName().c_str());')
    if new in content:
        print("Inspector already shows selected entity name")
        return
    if old not in content:
        raise RuntimeError("Cannot find Inspector entity label in expected format")
    file.write_text(content.replace(old, new, 1), encoding="utf-8")
    print("Inspector now displays selected entity name")


def update_docs() -> None:
    file = ROOT / "docs/scene-format.md"
    if not file.exists():
        return
    content = file.read_text(encoding="utf-8")
    old = "**Entities without a sprite:** `sprite` is required on every entity."
    changes = {
        old: "**Entities without a sprite:** `sprite` is optional; `transform` is required. `animation` requires `sprite`.",
        "> **Status:** matches the `phase3` branch.": "> **Status:** updated for the Phase 4 editor scene format.",
        "> **Not wired into startup yet:** the running demo still builds its scene in code (`createGameScene()` in `src/core/game.cpp`). Loading this file at startup is part of the integration work.":
            "> **Integrated:** `createGameScene()` loads the Phase 3 demo JSON at startup, and the editor can Open, Save, and Save As scene files.",
        "| `sprite`    | object | yes": "| `sprite`    | object | no ",
        '| Missing component                       | no `"transform"` / `"sprite"`':
            '| Missing component                       | no `"transform"`',
        "each entity's `name`, `Transform`, `Sprite` (handle → path with `AssetManager::getPath`), and, if present, `ColliderComponent` and `AnimationComponent`.":
            "each entity's `name`, required `Transform`, and optional `Sprite` (handle → path with `AssetManager::getPath`), `ColliderComponent`, and `AnimationComponent`.",
        "Requirements: every entity must have a `Transform` and a `Sprite` whose texture was loaded through the same `AssetManager`; otherwise `save` throws.":
            "Requirements: every entity must have a `Transform`. `Sprite` is optional; if present, its texture must be known to the `AssetManager`. An `AnimationComponent` requires a `SpriteComponent`.",
    }
    revised = content
    for before, after in changes.items():
        revised = revised.replace(before, after)
    if revised != content:
        file.write_text(revised, encoding="utf-8")
        print("Updated outdated Phase 3 documentation statements")


def main() -> None:
    if not (ROOT / "src/platform/input.cpp").is_file():
        raise RuntimeError("Extract this ZIP into the project root (alongside CMakeLists.txt) first.")
    update_test(
        "tests/scene/test_scene_serializer.cpp",
        "SavingUnsupportedEntityFailsAndKeepsPreviousFile",
        '''TEST_F(SceneSerializerTest, SavingEntityWithoutSpriteRoundTrips) {
    Scene scene("Sprite optional");
    auto& entity = scene.createEntity();
    entity.addComponent<TransformComponent>();
    entity.setName("Empty visual");
    const auto destination = tempDir / "sprite-optional.json";

    ASSERT_NO_THROW(serializer.save(scene, destination, *assets));
    auto loaded = serializer.load(destination, *assets);
    ASSERT_EQ(loaded->getEntities().size(), 1u);
    EXPECT_EQ(loaded->getEntities()[0]->getName(), "Empty visual");
    EXPECT_NE(loaded->getEntities()[0]->getComponent<TransformComponent>(), nullptr);
    EXPECT_EQ(loaded->getEntities()[0]->getComponent<SpriteComponent>(), nullptr);
}''',
        "SavingEntityWithoutSpriteRoundTrips",
    )
    update_test(
        "tests/scene/test_scene_serializer_contract.cpp",
        "RejectsMissingSpriteWithEntityContext",
        '''TEST_F(SceneSerializerContractTest, AcceptsMissingSpriteAndPreservesAbsence) {
    const std::string json = R"({"version":1,"name":"No sprite","entities":[{
        "transform":{"x":0,"y":0,"width":20,"height":20}}]})";
    auto scene = serializer.load(WriteText("no-sprite.json", json), *assets);
    ASSERT_EQ(scene->getEntities().size(), 1u);
    EXPECT_EQ(scene->getEntities()[0]->getComponent<SpriteComponent>(), nullptr);
    const auto saved = temporaryDirectory / "no-sprite-saved.json";
    ASSERT_NO_THROW(serializer.save(*scene, saved, *assets));
    const auto result = nlohmann::json::parse(ReadAll(saved));
    EXPECT_FALSE(result["entities"][0].contains("sprite"));
}''',
        "AcceptsMissingSpriteAndPreservesAbsence",
    )
    update_input()
    update_inspector_label()
    update_docs()
    print("\nPhase 4 test integration applied. Reconfigure CMake and run CTest.")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
