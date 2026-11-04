# Phase 3 Verification

> **Status:** template for the `phase3` branch. Every box must be ticked, or the failure linked to a fix on the owning feature.

## 1. Clean checkout

Run from a **fresh clone**, not your working folder.

```sh
git clone --branch phase3 https://github.com/Destan705/Destan_Yassmine_Cool_Amazing_Project.git phase3-check
cd phase3-check
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
ctest --test-dir build -N          # lists every registered test
```

| Check | Destan (macOS) | Yassmine (Windows) |
| --- | --- | --- |
| Commit hash (`git rev-parse --short HEAD`) | | |
| Configure + build succeeds | ☐ | ☐ |
| All `ctest` tests pass (count: ___) | ☐ | ☐ |
| Every test executable appears in `ctest -N` | ☐ | ☐ |
| `clang-format --dry-run --Werror` on `src/`, `include/`, `tests/` is clean | ☐ | ☐ |
| Demo runs: two sprites sharing a texture, one animated | ☐ | ☐ |
| Space plays the test sound once per press (or a clear warning without a sound device) | ☐ | ☐ |
| Platform limitations noticed | | |

**CI:** PR link: ____ — status ☐ green (build, clang-tidy, tests).

Note: `engine_tests` is the only test executable. The old `game_tests` target is commented out in `tests/CMakeLists.txt` because `test_game.cpp` was removed with the Phase 1 demo. Confirm that's intended, or delete the commented block.

## 2. Independent use of the APIs

| Task | Who | Done | How (file / API used) |
| --- | --- | --- | --- |
| Add a second animated entity via data / public APIs | Yassmine | ☐ | |
| Add a trigger using `ColliderComponent` / public APIs (e.g. an overlapping collider in `createGameScene()` or a scene file; check the log shows `Collision detected between Entity …`) | Destan | ☐ | |
| Neither change touched SDL polling or the engine loop | both | ☐ | |

## 3. Teach-back

Each person explains each topic in their own words; tick when the other agrees it was correct.

| Topic | Destan | Yassmine |
| --- | --- | --- |
| Texture cache ownership (Renderer owns, AssetManager maps, Sprite holds a handle) | ☐ | ☐ |
| Saved paths vs runtime handles | ☐ | ☐ |
| Failed-load behaviour (no half scene, old scene stays, no event) | ☐ | ☐ |
| AABB edge rules (touching edges/corners is not a collision) | ☐ | ☐ |
| Source vs destination rectangles | ☐ | ☐ |
| Animation timing (`elapsed`, `frameDuration`, leftover time, loop vs hold) | ☐ | ☐ |
| Sound trigger timing (`wasKeyPressed`, not `isKeyHeld`) | ☐ | ☐ |
| Shutdown order (scene → audio → AssetManager → Renderer → window → SDL) | ☐ | ☐ |

## 4. Known limitations

| Limitation | Effect |
| --- | --- |
| **Retained textures** | Textures are never unloaded until the Renderer is destroyed; switching scenes does not free them |
| **Scene-local IDs** | Entity IDs restart at 1 in every scene and are not saved; never store an ID across a scene switch |
| **Detection-only collision** | Overlaps are reported; objects are not pushed apart or blocked |
| **No enter/exit events** | A pair is reported every frame while overlapping; gameplay must compare frames to detect "entered" |
| **No continuous collision** | Fast, thin objects can pass through each other between frames |
| **One audio playback** | Only one sound plays at a time; playing again restarts it |
| **Short WAVs only** | WAVs longer than 5 seconds are rejected; total cached audio is capped |
| **Scene-content saves** | Saves store scene layout, not a gameplay checkpoint (no animation progress, no game state) |
| **BMP only** | Textures load through `SDL_LoadBMP` |
| **Frame bounds checked at draw time** | An out-of-range animation frame is skipped with a warning, not rejected on load |
| **Demo not loaded from JSON** | `createGameScene()` builds the demo in code with working-directory-relative paths; run from the repo root |
