# Other Project Files

> **Coverage:** Verified · **Checked against:** `phase3` @ `ca2b728`

**In one sentence:** The build scripts, CI, code-style tools, assets and Git settings that support the engine code.

## Build and project setup

| File/folder | What it does | Details |
| --- | --- | --- |
| [`CMakeLists.txt`](../CMakeLists.txt) (root) | C++20, downloads dependencies, adds `src` and `tests` | [Below](#root-cmakeliststxt) |
| [`src/CMakeLists.txt`](../src/CMakeLists.txt) | Builds the `game_engine` executable | [Below](#srccmakeliststxt) |
| [`tests/CMakeLists.txt`](../tests/CMakeLists.txt) | Builds `engine_tests` (`game_tests` is commented out) | [Below](#testscmakeliststxt) |
| [`.github/workflows/ci.yml`](../.github/workflows/ci.yml) | CI: build, clang-tidy, tests | [Below](#githubworkflowsciyml) |
| [`.clang-format`](../.clang-format) | Code style | [Below](#clang-format) |
| [`.clang-tidy`](../.clang-tidy) | Static-analysis checks | [Below](#clang-tidy) |
| [`.gitignore`](../.gitignore) | Ignored files | `build/`, `CMakeFiles/`, `site/`, `build-mingw/` (Windows MinGW build folder) |
| `assets/` | Images, a sound and a scene file used by the demo and tests | [Below](#assets) |
| `.DS_Store` | macOS Finder metadata, **committed by accident** | Can be removed and added to `.gitignore` |
| Docs tooling: `mkdocs.yml`, `check_doc_links.py`, `CLAUDE_DOCS_PROMPT.md`, `.doc-templates/` | This handbook | [Documentation guide](contributing.md) |

<a id="root-cmakeliststxt"></a>
### Root `CMakeLists.txt`

| Item | Value | Why it matters |
| --- | --- | --- |
| `cmake_minimum_required` | 3.20 | Needed for `FetchContent_MakeAvailable` |
| `ENGINE_INCLUDE_DIRS` | `include/` and its 6 subfolders | Lets code write `#include "scene.h"` without folder paths. Phase 3 headers (`asset_manager.h`, `audio_service.h`, `colliderSystem.h`, `scene_serializer.h`) sit directly in `include/`. |
| `CMAKE_EXPORT_COMPILE_COMMANDS` | ON | Creates `build/compile_commands.json` for clang-tidy and editors |
| `FetchContent` SDL3 | `release-3.4.16` | Downloaded and built from source on first configure |
| `FetchContent` googletest | `v1.15.2` | Test framework |
| `FetchContent` nlohmann/json | `v3.11.3` | JSON parsing for the [SceneSerializer](code/scene-serializer.md); target `nlohmann_json::nlohmann_json` |
| `CMAKE_RUNTIME_OUTPUT_DIRECTORY` | `build/bin` | All executables end up in `build/bin/` |
| C++ standard | 20, required | |
| `enable_testing()` | | Turns on CTest |

<a id="srccmakeliststxt"></a>
### `src/CMakeLists.txt`

Builds **`game_engine`** from every `.cpp` in `src/` (each must be listed **by hand**, so add new files here). It links `SDL3::SDL3` and `nlohmann_json::nlohmann_json`. Phase 3 added `ecs/colliderComponent.cpp`, `colliderSystem.cpp`, `asset_manager.cpp`, `audio_service.cpp` and `scene_serializer.cpp`. Post-build steps copy `SDL3.dll` next to the executable (Windows only) and copy `assets/` next to the executable.

<a id="testscmakeliststxt"></a>
### `tests/CMakeLists.txt`

| Target | Contents | Registered with CTest? |
| --- | --- | --- |
| `engine_tests` | All 15 test files, **plus** every engine `.cpp` except `main.cpp` listed again (`../src/...`); links GoogleTest, SDL3 and nlohmann/json; copies `assets/` next to the executable | Yes, via `gtest_discover_tests` (`PRE_TEST`, working dir = project root, 30 s timeout) |
| `game_tests` | **Commented out.** `test_game.cpp` was removed with the Phase 1 cat/chef game. | — |

**Gotcha:** a new engine `.cpp` must be added to **both** `src/CMakeLists.txt` and the `engine_tests` list, otherwise you get linker "undefined symbols" errors in the tests.

<a id="githubworkflowsciyml"></a>
### `.github/workflows/ci.yml`

| Question | Answer |
| --- | --- |
| Triggers | Push to `main`; pull requests into `main`, `phase1`, `phase2`, `phase3` |
| Runner | `ubuntu-latest` |
| Steps | Checkout → install SDL's Linux build dependencies + Xvfb → `cmake -B build` → `cmake --build build` → `clang-tidy` on `src/**/*.cpp` → `xvfb-run … ctest` |
| Test environment | `SDL_VIDEO_DRIVER=x11`, `SDL_AUDIO_DRIVER=dummy`, `LIBGL_ALWAYS_SOFTWARE=1`, inside a virtual X display |
| Success | Build passes, clang-tidy reports no errors, all CTest tests pass |

**Not checked by CI:** `clang-format` (run `clang-format --dry-run --Werror` locally) and pull requests into feature branches. clang-tidy only scans `src/`, not `tests/`.

<a id="clang-format"></a>
### `.clang-format`

LLVM style, 4-space indent, no tabs, 100-column limit, `int* p` pointer style, braces on the same line, no one-line functions, includes sorted. Format with `clang-format -i <files>`.

<a id="clang-tidy"></a>
### `.clang-tidy`

Enables `bugprone-*`, `performance-*`, `modernize-use-nullptr`, `modernize-use-override` and `readability-braces-around-statements`. Warnings are **not** errors (`WarningsAsErrors: ''`). Locally on macOS it may fail with system-header errors, so rely on CI.

<a id="assets"></a>
### `assets/`

| File | Format | Used by | If missing |
| --- | --- | --- | --- |
| `test.bmp` | BMP | [`initializePlatform`](code/platform.md#initializeplatform); `phase3-demo.json` entities 1–2 | Start-up fails, or `engine()` logs an error and shuts down |
| `anim_sheet.bmp` | 128×32 BMP, 24-bit, 4 frames of 32×32 side by side (red, orange, teal, blue, with 1–4 white dots) | `phase3-demo.json` entity 3 (the player) + [AnimationComponent](code/animation-component.md) | `engine()` logs an error and shuts down |
| `audio/test.wav` | WAV, 16-bit, 4 channels, 22 050 Hz, about 4.8 s (just under the 5 s limit) | [`engine`](code/engine.md#engine) via [AudioService](code/audio-service.md), when the player enters the trigger | Warning logged; the trigger is silent |
| `scenes/phase3-demo.json` | Version-1 scene file: decoration, trigger (collider), player (animation + collider). See [the demo scene](code/game.md#the-demo-scene). | [`createGameScene`](code/game.md#creategamescene) at start-up; [`LoadsRealPhase3DemoThroughAssetManager`](tests/test-scene-serializer-contract.md#sceneserializercontracttest-loadsrealphase3demothroughassetmanager) | The game can't start; that test fails |
| `scenes/phase3-demo-saved.json` | **Not in the repo.** Created by F5 in `build/bin/assets/scenes/`; read by F9. | [`engine`](code/engine.md#engine) | F9 logs "No saved Scene found. Press F5 first." |

Images: only **BMP** (`SDL_LoadBMP`). Sounds: only **WAV** (`SDL_LoadWAV`). Texture paths inside scene files are relative to `assets/`.

## What about `build/`?

`build/` is the **generated** CMake folder: the cache, downloaded SDL/googletest sources (`_deps/`), object files, `compile_commands.json`, and executables in `build/bin/` (`game_engine`, `engine_tests`, plus a copy of `assets/`). It's ignored by Git. Delete it and reconfigure if things get weird.

## Environment & build quick reference

**Prerequisites:** C++20 compiler, CMake ≥ 3.20, Git, internet access for the first configure. Linux also needs SDL's dev packages (see `ci.yml`).

```sh
cmake -S . -B build                 # configure
cmake --build build                 # build
ctest --test-dir build --output-on-failure
./build/bin/game_engine             # W/A/S/D move, F5 save, F9 reload
```

**Next:** [Architecture](architecture/index.md) · [Test index](tests/index.md)
