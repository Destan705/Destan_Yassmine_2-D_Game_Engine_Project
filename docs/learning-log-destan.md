# Learning Log: Destan (Phase 3)

> Draft: read it through and change anything that doesn't sound like you. One concrete contribution per subsystem, plus what I learned doing it.

## Rendering: source rectangles

**What I built:** `Renderer::DrawTexture` takes an optional source rectangle (`std::optional<Bounds> src`). It checks that the rectangle is inside the texture using `SDL_GetTextureSize`, and passes it to `SDL_RenderTexture`. `Sprite` stores it and exposes `sourceSet`.

**What I learned:**
- **Source vs destination:** the source is *which part of the image*, in texture pixels; the destination is *where on screen and how big*. One texture can show many different frames.
- `std::optional` with `has_value()` and `->`, and that default arguments belong only in the header.
- SDL functions often return a `bool` and write results into **output parameters** (`&w`, `&h`).

## Animation

**What I built:** `AnimationComponent` with a frame list, seconds-per-frame, loop or hold-last-frame, and `pause`/`resume`/`restart`. Each `update(dt)` picks the current frame and calls `Sprite::sourceSet`.

**What I learned:**
- **`dt` vs `elapsed` vs `duration`:** adding `dt` to `elapsed` and using a `while` loop that subtracts `duration` keeps the leftover time, so timing doesn't drift and big frames can skip ahead.
- The pause check must come **first** in `update`, otherwise time keeps building up while paused.
- **Low coupling:** Animation knows about Sprite, but Sprite doesn't know about Animation.

## AssetManager

**What I built:** `Renderer::getTextureDimensions(handle)`, which returns the texture's width and height, and finished `AssetManager::getOriginalDimensions` with it.

**What I learned:**
- A function can only return one thing, so `std::pair` bundles two values.
- `throw` leaves a function immediately, like `return`, so nothing after it runs.
- `float &w;` declares a *reference*, which must be initialised immediately; `&w` in a call means *the address of* `w`.

## Scene serialization

**What I built:** the version-1 JSON format and `SceneSerializer`:
- `load` validates everything (syntax, duplicate keys, version, unknown fields, types, values) and builds a **temporary** `Scene` that the caller activates.
- `save` writes a temp file and then replaces the real one, so a failed save never damages the old file.
- I added `Scene::getEntities() const` so `save` can read a scene without changing it.

**What I learned:**
- JSON's four building blocks: objects, arrays, strings and numbers/bools.
- **Order matters with `||`:** checking `contains` first stops the code reading a key that isn't there.
- **Variable scope:** declaring `data` inside the `try { }` makes it disappear at the `}`; declaring it again inside creates a *second* variable.
- **Lambdas:** a small function written inline that can use local variables through `[&]`. I used one as the parser callback to catch duplicate keys, with a `vector` of `set`s (one set per open `{`).
- `.` for objects and references, `->` for pointers and `unique_ptr`.
- Never use a pointer before checking it isn't `nullptr`, for example `getComponent<Sprite>()->getTexture()` crashes if there's no Sprite.

## Build, Git and teamwork

**What I did:** pinned nlohmann/json in CMake and linked it to both targets; merged the phase2 fixes, AssetManager and animation into my branch and resolved the conflicts by keeping both sides.

**What I learned:**
- `git switch -c` creates a branch from **wherever I currently am**, so give the start point explicitly (`git switch -c new-branch origin/phase2`).
- A merge conflict where both sides *added* different things usually means **keep both**.
- **Save before committing:** twice I committed an old version because VS Code hadn't saved. Auto Save is now on.
- Tests caught two real bugs in what I had pushed (load stopping after the first entity, and save skipping animation).

## Collision and audio

Yassmine built these. To be filled in after I add a trigger with `ColliderComponent` and complete the teach-back for the verification ticket.
