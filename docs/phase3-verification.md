# Phase 3 Verification

> **Status:** template. Fill in after the integration ticket is merged. Every box must be ticked, or the failure linked to a fix on the owning feature.

## 1. Clean checkout

Run from a **fresh clone**, not your working folder.

```sh
git clone --branch <PR-target-branch> https://github.com/Destan705/Destan_Yassmine_Cool_Amazing_Project.git phase3-check
cd phase3-check
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
ctest --test-dir build -N          # lists every registered test
./build/bin/game_tests             # until it is registered with CTest
```

| Check | Destan (macOS) | Yassmine (Windows) |
| --- | --- | --- |
| Commit hash (`git rev-parse --short HEAD`) | | |
| Configure + build succeeds | ☐ | ☐ |
| All `ctest` tests pass (count: ___) | ☐ | ☐ |
| Every test executable appears in `ctest -N` (incl. `game_tests`) | ☐ | ☐ |
| `clang-format --dry-run --Werror` on `src/`, `include/`, `tests/` is clean | ☐ | ☐ |
| Demo runs: two sprites sharing a texture, one animated | ☐ | ☐ |
| Platform limitations noticed | | |

**CI:** PR link: ____ — status ☐ green (build, clang-tidy, tests).

Known gap to fix before ticking: `game_tests` is not registered with CTest (`tests/CMakeLists.txt` only calls `gtest_discover_tests(engine_tests)`).

## 2. Independent use of the APIs

| Task | Who | Done | How (file / API used) |
| --- | --- | --- | --- |
| Add a second animated entity via data / public APIs | Yassmine | ☐ | |
| Add a trigger using `ColliderComponent` / public APIs | Destan | ☐ | |
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
| **No continuous collision** | Fast, thin objects can pass through each other between frames |
| **One audio playback** | Only one sound plays at a time |
| **Scene-content saves** | Saves store scene layout, not a gameplay checkpoint (no animation progress, no game state) |
| **BMP only** | Textures load through `SDL_LoadBMP` |
| **Frame bounds checked at draw time** | An out-of-range animation frame is skipped with a warning, not rejected on load |
