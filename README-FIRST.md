# Phase 4: final CTest timeout fix

## Replace
Extract this ZIP into the repository root, allowing it to overwrite `tests/core/test_engine.cpp`.

## Why
The old `EngineTest.WhenWindowCreationFailsEngineReturnsAndCleansUp` relied on `SDL_HINT_VIDEO_DRIVER=dummy` making window creation fail. After the Phase 4 window change removed `SDL_WINDOW_OPENGL`, the dummy driver can create the window. The test then launched a working engine loop without a quit event and timed out.

This patch removes only the invalid test case. The existing tests for startup failure, renderer creation failure, window close and restart remain. It intentionally does *not* claim to test SDL_CreateWindow failure; test that later with an injected window-creation failure rather than relying on a driver-specific assumption.

## Verify
```
cmake --build build-mingw -j 4
ctest --test-dir build-mingw --output-on-failure
```

Note: CTest discovers tests on build; the new total should be one fewer (251 rather than 252), provided no other tests were added.
