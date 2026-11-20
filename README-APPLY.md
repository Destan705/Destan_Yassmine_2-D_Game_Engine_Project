# Phase 4 Editor — Test & Review Bundle

**Base branch:** `phase4` at commit `e3dfd4bf4850a193057bc3e8414ee9827516bb69` (November 19, 2026).

## How to install (no copying individual files)

1. **Create a new working branch** from `phase4` before extracting (`git switch phase4` then `git switch -c test/phase4-editor-verification`), and commit/stash any local changes.
2. Extract the ZIP **into the project root**, alongside the existing `CMakeLists.txt`. Allow overwriting of the few listed files. Do **not** extract to a nested folder.
3. Run `python phase4_apply.py` **once** from the project root. This updates two outdated Scene Serializer tests and input routing without replacing entire existing source files. It can be safely rerun.
4. Reconfigure, build, and run both suites:

   Windows CMD:
   ```cmd
   python phase4_apply.py
   cmake -S . -B build-mingw -DENGINE_EDITOR=ON
   cmake --build build-mingw -j 4
   ctest --test-dir build-mingw --output-on-failure
   ```

   To check the editor-OFF build independently:
   ```cmd
   cmake -S . -B build-no-editor -DENGINE_EDITOR=OFF
   cmake --build build-no-editor -j 4
   ctest --test-dir build-no-editor --output-on-failure
   ```

## What is included

- `tests/editor/test_editor_state.cpp` — P4-109: selection IDs, scene generations, pause, dirty state, pending scene/deletion operations
- `tests/editor/test_editor_input.cpp` — P4-109: gameplay/UI keyboard capture transitions, held/press/release edges, focus loss, shortcuts
- `tests/editor/test_editor_input_integration.cpp` — P4-109: actual SDL event queue press/release and quit handling (non-ImGui harness)
- `tests/editor/test_editor_actions.cpp` — P4-115: Transform and Collider validation, no-op edits, entity creation/renaming
- `tests/editor/test_inspector_panel.cpp` — P4-115: component add/remove data semantics (not direct ImGui widget automation)
- `tests/editor/test_collision_overlay.cpp` — P4-133: offsets, skip rules, colors, overlaps, toggle, stale-scene isolation
- `tests/editor/test_log_buffer.cpp` — P4-127: 1,000-entry cap, 4,096-byte truncation, ordering, clear and filters
- `tests/editor/test_console_commands.cpp` — P4-127: exactly four commands, argument validation and pause/resume effects
- `tests/editor/test_logger_subscription.cpp` — P4-127: listener lifecycle and preserving terminal output
- Testable production changes to input capture, collider-outline collection, console command parsing, and editor state
- Updated `.github/workflows/ci.yml` to exercise editor ON and OFF on push/PR to `phase4`
- Replaces **two obsolete Sprite-required expectations** (rather than deleting valuable old serializer tests)

The existing Phase 1–3 tests remain in place and run in both configurations.

## Important manual checks and known gaps

This package was code-reviewed against the above GitHub commit. **It could not be built here**, because this environment cannot download your repository or its SDL3/GoogleTest dependencies. The test suite is not claimed to pass until you run it locally/through CI.

### P4-109 — Editor foundation/input
- **Review:** SDL event polling, ImGui event forwarding, paused simulation, lifecycle hooks and editor toggle exist. Current input handling used physical SDL keyboard state for held keys but event-level gating for press/release. The bundle integrates a stateful gameplay input gate to avoid stuck keys/false edges when the UI captures keyboard input.
- **Still manually verify:** clicking/typing while player is moving, resizing, closing window while editing, editor enabled/disabled builds on Windows/macOS/Linux. The ImGui SDL event backend still relies on current IO capture flags; validate focus changes on the real app. The previous CI did not build the editor or run for `phase4`; this bundle adds an ON/OFF CI matrix, but it still needs an actual GitHub Actions run.

### P4-115 — Inspector
- **Review:** existing Transform edit actions check finite values and positive size; Sprite/Collider display/edit and component add/remove work through Inspector. The panel currently labels selection by ID, not the entity's name (Hierarchy does show names). **Missing:** automated ImGui widget buffer reset/disabled-state tests; these need a GUI harness or manual verification.
- **Still manually verify:** selection switch, edit-then-resume buffer discard, disabled edits while running, bad Sprite file error feedback.

### P4-127 — Console
- **Review:** logging subscriptions, buffer capacity/truncation, filters and help/clear/pause/resume exist. The package extracts the command/filter logic into testable functions and uses them in the panel. **Gap:** the logger's global listener vector and the Console's log buffer are not synchronized for concurrent caller threads; do not assume thread safety. Consider one-thread ownership or synchronization in a follow-up.
- **Still manually verify:** filters and scrolling in the UI, repeated editor startup/shutdown, command typing doesn't move player, output still appears in terminal.

### P4-133 — Collider overlay
- **Review:** draw hook, current-scene collision snapshot and visibility toggle exist; green/red/yellow precedence matches the ticket. The package extracts outline collection so geometry, selection, skips, and colors can be tested. **Still manually verify:** actual rendered line width/viewport alignment, resize/scale, and renderer state restoration (tests are geometry-level, not screenshot tests).

## Why `phase4_apply.py` is required

Two old tests are **inside large existing test source files** also containing valuable legacy tests. The script updates only those two named cases; overwriting whole files would risk losing unrelated tests changed by your teammate. It also preserves the existing 200+ line SDL key-scancode mapping while fixing input routing in place.

## Commit suggestion

```cmd
git add src include tests docs/scene-format.md phase4_apply.py
git commit -m "Add Phase 4 editor regression tests and refresh legacy contracts"
git push -u origin test/phase4-editor-verification
```
