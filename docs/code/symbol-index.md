# Symbol Index

**An alphabetical jump list for every class, method, function, field, enum and constant.**

> Checked against `phase3` @ `ca2b728`. Alphabetised by the name after any `Class::` prefix is ignored.

| Symbol (exact name) | Kind | One-line purpose | Jump to explanation |
| --- | --- | --- | --- |
| `Entity::addComponent<T>(Args&&...)` | template method | Create and attach a component | [Jump](entity.md#entity-addcomponent) |
| `Color::alpha` | field | Opacity 0–255 | [Jump](render-types.md#color-alpha) |
| `AnimationComponent` | class | Sprite-sheet animator | [AnimationComponent](animation-component.md#class-animationcomponent) |
| `AnimationComponent::AnimationComponent(const std::vector<Bounds>&, float, bool)` | constructor | Validate and store frames | [Jump](animation-component.md#animationcomponent-animationcomponent) |
| `AssetManager` | class | Path ↔ texture-handle cache | [Jump](asset-manager.md#class-assetmanager) |
| `assetManager` (platform global) | global | Owned AssetManager | [Jump](platform.md#platform-assetmanager) |
| `AssetManager::AssetManager(Renderer&, std::filesystem::path)` | constructor | Bind renderer and root | [Jump](asset-manager.md#assetmanager-assetmanager) |
| `AssetManager::assetRoot` | field | Root folder | [Jump](asset-manager.md#assetmanager-assetroot) |
| `AudioService` | class | WAV cache and player | [Jump](audio-service.md#class-audioservice) |
| `AudioService::AudioService(std::filesystem::path)` | constructor | Open device stream | [Jump](audio-service.md#audioservice-audioservice) |
| `AudioService::~AudioService()` | destructor | Close device stream | [Jump](audio-service.md#audioservice-destructor) |
| `beginFrame()` | free function | Old API: clear to black | [Jump](platform.md#beginframe) |
| `Color::blue` | field | Blue 0–255 | [Jump](render-types.md#color-blue) |
| `Bounds` | struct | Rectangle in pixels | [Jump](render-types.md#struct-bounds) |
| `CachedSound (audio_service.cpp)` | private struct | Samples + format | [Jump](audio-service.md#internals) |
| `checkKnownKeys(...) (scene_serializer.cpp)` | private function | Reject unknown fields | [Jump](scene-serializer.md#helpers) |
| `Renderer::ClearScreen(const Color&)` | method | Fill the frame with a colour | [Jump](renderer.md#renderer-clearscreen) |
| `ColliderComponent` | class | Local hitbox | [Jump](collider-component.md#class-collidercomponent) |
| `ColliderComponent::ColliderComponent(float, float, float, float, bool)` | constructor | Validate and store | [Jump](collider-component.md#collidercomponent-collidercomponent) |
| `ColliderSystem` | class | Overlap detector | [Jump](collider-system.md#class-collidersystem) |
| `ColliderSystem::CollisionPair` | type alias | Pair of entity IDs | [Jump](collider-system.md#collidersystem-collisionpair) |
| `Color` | struct | RGBA colour | [Jump](render-types.md#struct-color) |
| `Component` | class | Base for all components | [Jump](component.md#class-component) |
| `Component::~Component()` | virtual destructor | Proper subclass clean-up | [Jump](component.md#component-destructor) |
| `Entity::components` | field | Owned components | [Jump](entity.md#entity-components) |
| `Scene::createEntity()` | method | Make a new entity | [Jump](scene.md#scene-createentity) |
| `createGameScene(AssetManager&)` | free function | Load the demo scene from JSON | [Jump](game.md#creategamescene) |
| `AnimationComponent::currentFrame` | field | Frame being shown | [Jump](animation-component.md#animationcomponent-currentframe) |
| `SceneManager::currentScene` | field | Owned active scene | [Jump](scene-manager.md#scenemanager-currentscene) |
| `currentTexture` (platform global) | global | Start-up texture handle | [Jump](platform.md#platform-currenttexture) |
| `Sprite::currentTexture` | field | Sprite's texture handle | [Jump](sprite.md#sprite-currenttexture) |
| `ColliderSystem::detectCollisions(Scene&) const` | method | Find overlapping pairs | [Jump](collider-system.md#collidersystem-detectcollisions) |
| `EventDispatcher::dispatch(const SceneChangedEvent&)` | method | Call all listeners | [Jump](event-dispatcher.md#eventdispatcher-dispatch) |
| `drawFilledRect(Color, Bounds)` | free function | Old API: rectangle | [Jump](platform.md#drawfilledrect) |
| `Renderer::DrawFilledRect(const Color&, const Bounds&)` | method | Draw a solid rectangle | [Jump](renderer.md#renderer-drawfilledrect) |
| `drawTexture()` | free function | Old API: draw start-up texture | [Jump](platform.md#drawtexture) |
| `Renderer::DrawTexture(textureHandle, const Bounds&, std::optional<Bounds>)` | method | Draw texture, optionally cropped | [Jump](renderer.md#renderer-drawtexture) |
| `AnimationComponent::duration` | field | Seconds per frame | [Jump](animation-component.md#animationcomponent-duration) |
| `AnimationComponent::elapsed` | field | Time on current frame | [Jump](animation-component.md#animationcomponent-elapsed) |
| `ColliderComponent::enabled` | field | On/off | [Jump](collider-component.md#collidercomponent-enabled) |
| `engine()` | free function | Whole engine lifetime | [Jump](engine.md#engine) |
| `entities` (Scene) | field | Owned entities | [Jump](scene.md#scene-entities) |
| `Entity` | class | Game object owning components | [Jump](entity.md#class-entity) |
| `Entity::Entity(std::uint64_t)` | constructor | Create with ID | [Jump](entity.md#entity-entity) |
| `EventDispatcher` | class | Subscribe/notify hub | [Jump](event-dispatcher.md#class-eventdispatcher) |
| `SceneManager::eventDispatcher` | field | Borrowed dispatcher | [Jump](scene-manager.md#scenemanager-eventdispatcher) |
| `event` (input global) | global | SDL event scratch | [Jump](input.md#input-event) |
| `AnimationComponent::frames` | field | Source rectangles | [Jump](animation-component.md#animationcomponent-frames) |
| `SceneManager::getActiveScene()` | method | Access active scene | [Jump](scene-manager.md#scenemanager-getactivescene) |
| `getAssetRootPath()` | free function | `<exe folder>/assets` | [Jump](platform.md#getassetrootpath) |
| `Entity::getComponent<T>()` | template method | Find component by exact type | [Jump](entity.md#entity-getcomponent) |
| `AnimationComponent::getDuration() const` | method | Seconds per frame | [Jump](animation-component.md#animationcomponent-getduration) |
| `Scene::getEntities() const` | method | Read-only entity list | [Jump](scene.md#scene-getentities) |
| `AnimationComponent::getFrames() const` | method | Frame rectangles (const ref) | [Jump](animation-component.md#animationcomponent-getframes) |
| `ColliderComponent::getHeight() const` | method | Height | [Jump](collider-component.md#collidercomponent-getters) |
| `Entity::getId()` | method | Read ID | [Jump](entity.md#entity-getid) |
| `ColliderComponent::getOffsetX() const` | method | X offset | [Jump](collider-component.md#collidercomponent-getters) |
| `ColliderComponent::getOffsetY() const` | method | Y offset | [Jump](collider-component.md#collidercomponent-getters) |
| `AssetManager::getOriginalDimensions(textureHandle) const` | method | Texture width/height | [Jump](asset-manager.md#assetmanager-getoriginaldimensions) |
| `Component::getOwner()` | method | Owning entity | [Jump](component.md#component-getowner) |
| `AssetManager::getPath(textureHandle) const` | method | Handle → path | [Jump](asset-manager.md#assetmanager-getpath) |
| `getPlatformAssetManager()` | free function | The engine's AssetManager | [Jump](platform.md#getplatformassetmanager) |
| `AnimationComponent::getRepeat() const` | method | Loop flag | [Jump](animation-component.md#animationcomponent-getrepeat) |
| `Scene::getSceneName() const` | method | Read name | [Jump](scene.md#scene-getscenename) |
| `Renderer::getTextureDimensions(textureHandle)` | method | Texture width/height | [Jump](renderer.md#renderer-gettexturedimensions) |
| `Sprite::getTexture() const` | method | Texture handle | [Jump](sprite.md#sprite-gettexture) |
| `ColliderComponent::getWidth() const` | method | Width | [Jump](collider-component.md#collidercomponent-getters) |
| `getWorldBounds(Entity&) (colliderSystem.cpp)` | private function | World box or skip | [Jump](collider-system.md#helpers) |
| `Color::green` | field | Green 0–255 | [Jump](render-types.md#color-green) |
| `handleCollisions(const std::vector<CollisionPair>&, Scene&, bool&)` | free function | Did the player just enter the trigger? | [Jump](game.md#handlecollisions) |
| `EventDispatcher::Handler` | type alias | Listener function type | [Jump](event-dispatcher.md#eventdispatcher-handler) |
| `AssetManager::handleToPath` | field | Handle → path map | [Jump](asset-manager.md#assetmanager-handletopath) |
| `Renderer::hasWarned` | field | Warn-once flag | [Jump](renderer.md#renderer-haswarned) |
| `Bounds::h` | field | Height | [Jump](render-types.md#bounds-h) |
| `ColliderComponent::height` | field | Hitbox height | [Jump](collider-component.md#collidercomponent-height) |
| `Transform::height` | field | Height | [Jump](transform.md#transform-height) |
| `EventDispatcher::idCounter` | field | Next listener ID | [Jump](event-dispatcher.md#eventdispatcher-idcounter) |
| `Scene::iDcounter` | field | Next entity ID | [Jump](scene.md#scene-idcounter) |
| `Entity::id` | field | Entity ID | [Jump](entity.md#entity-id) |
| `AudioService::Impl` | private struct | Hidden state (pimpl) | [Jump](audio-service.md#internals) |
| `initializeInput()` | free function | Get SDL keyboard array | [Jump](input.md#initializeinput) |
| `initializePlatform()` | free function | Start SDL, window, renderer | [Jump](platform.md#initializeplatform) |
| `AudioService::isAvailable() const` | method | Device opened? | [Jump](audio-service.md#audioservice-isavailable) |
| `ColliderComponent::isEnabled() const` | method | On? | [Jump](collider-component.md#collidercomponent-isenabled) |
| `isKeyHeld(Key)` | free function | Key currently down? | [Jump](input.md#iskeyheld) |
| `Key` | enum class | Engine key names | [Jump](input.md#input-key) |
| `keystate` | global | SDL keyboard array | [Jump](input.md#input-keystate) |
| `EventDispatcher::ListenerId` | type alias | Subscription ID | [Jump](event-dispatcher.md#eventdispatcher-listenerid) |
| `loadPlatformTexture(const char*)` | free function | Load BMP via platform (unused by the game) | [Jump](platform.md#loadplatformtexture) |
| `SceneSerializer::load(std::filesystem::path, AssetManager&)` | method | File → new Scene | [Jump](scene-serializer.md#sceneserializer-load) |
| `AssetManager::loadTexture(std::filesystem::path)` | method | Cached texture load | [Jump](asset-manager.md#assetmanager-loadtexture) |
| `Renderer::LoadTexture(const char*)` | method | Load BMP into texture | [Jump](renderer.md#renderer-loadtexture) |
| `AudioService::loadWav(std::filesystem::path)` | method | Load and cache a WAV | [Jump](audio-service.md#audioservice-loadwav) |
| `logger_level` | enum class | Info / Warning / Error | [Jump](logger.md#logger-level) |
| `loggerLevelToString(logger_level)` | free function | Level → label | [Jump](logger.md#loggerleveltostring) |
| `logMessage(logger_level, const std::string&)` | free function | Print labelled message | [Jump](logger.md#logmessage) |
| `main()` | free function | Program entry | [Jump](engine.md#main) |
| `maxCacheSize / maxDurationSeconds / maxWavSize` | constants | Audio limits | [Jump](audio-service.md#internals) |
| `SceneChangedEvent::newSceneName` | field | New scene name | [Jump](event-dispatcher.md#scenechangedevent-newscenename) |
| `AssetManager::normalizePath(std::filesystem::path) const` | private method | Validate and clean a path | [Jump](asset-manager.md#assetmanager-normalizepath) |
| `normalizePath(...) (audio_service.cpp)` | private function | Path check, no throw | [Jump](audio-service.md#internals) |
| `ColliderComponent::offsetX` | field | X offset from Transform | [Jump](collider-component.md#collidercomponent-offsetx) |
| `ColliderComponent::offsetY` | field | Y offset from Transform | [Jump](collider-component.md#collidercomponent-offsety) |
| `SceneChangedEvent::oldSceneName` | field | Old scene name | [Jump](event-dispatcher.md#scenechangedevent-oldscenename) |
| `overlaps(...) (colliderSystem.cpp)` | private function | Strict AABB test | [Jump](collider-system.md#helpers) |
| `Component::owner` | field | Back-pointer to entity | [Jump](component.md#component-owner) |
| `AssetManager::pathToHandle` | field | Path → handle map | [Jump](asset-manager.md#assetmanager-pathtohandle) |
| `AnimationComponent::pause()` | method | Freeze animation | [Jump](animation-component.md#animationcomponent-pause) |
| `AnimationComponent::paused` | field | Paused flag | [Jump](animation-component.md#animationcomponent-paused) |
| `AudioService::play(SoundId)` | method | Play from the start | [Jump](audio-service.md#audioservice-play) |
| `presentFrame()` | free function | Old API: present | [Jump](platform.md#presentframe) |
| `Renderer::Present()` | method | Show frame | [Jump](renderer.md#renderer-present) |
| `pressedThisFrame` | global | Keys pressed this frame | [Jump](input.md#input-pressedthisframe) |
| `processPlatformEvents()` | free function | Pump events; false on quit | [Jump](input.md#processplatformevents) |
| `readFiniteNumber(...) (scene_serializer.cpp)` | private function | Finite number or throw | [Jump](scene-serializer.md#helpers) |
| `readNumber(...) (scene_serializer.cpp)` | private function | Number or throw | [Jump](scene-serializer.md#helpers) |
| `rectangle (colliderSystem.cpp)` | private struct | left/right/top/bottom | [Jump](collider-system.md#helpers) |
| `Color::red` | field | Red 0–255 | [Jump](render-types.md#color-red) |
| `releasedThisFrame` | global | Keys released this frame | [Jump](input.md#input-releasedthisframe) |
| `Component::render(Renderer&)` | virtual method | Drawing hook | [Jump](component.md#component-render) |
| `renderEngineFrame(SceneManager&)` | free function | Draw active scene | [Jump](engine.md#renderengineframe) |
| `Entity::render(Renderer&)` | method | Render all components | [Jump](entity.md#entity-render) |
| `AssetManager::renderer` | field | Borrowed Renderer | [Jump](asset-manager.md#assetmanager-renderer) |
| `Renderer` | class | SDL renderer wrapper | [Jump](renderer.md#class-renderer) |
| `renderer` (platform global) | global | The engine's Renderer | [Jump](platform.md#platform-renderer) |
| `Renderer::renderer` | field | Owned `SDL_Renderer*` | [Jump](renderer.md#renderer-renderer-field) |
| `Renderer::Renderer(SDL_Window*)` | constructor | Create SDL renderer | [Jump](renderer.md#renderer-renderer) |
| `Renderer::~Renderer()` | destructor | Free textures and renderer | [Jump](renderer.md#renderer-destructor) |
| `renderPlatform(Scene&)` | free function | Clear, draw scene, present | [Jump](platform.md#renderplatform) |
| `Scene::render(Renderer&)` | method | Render all entities | [Jump](scene.md#scene-render) |
| `Sprite::render(Renderer&)` | method | Draw texture | [Jump](sprite.md#sprite-render) |
| `AnimationComponent::repeat` | field | Loop or hold last | [Jump](animation-component.md#animationcomponent-repeat) |
| `AnimationComponent::restart()` | method | Back to frame 0 | [Jump](animation-component.md#animationcomponent-restart) |
| `AnimationComponent::resume()` | method | Unfreeze | [Jump](animation-component.md#animationcomponent-resume) |
| `SceneSerializer::save(const Scene&, std::filesystem::path, const AssetManager&)` | method | Scene → file (safe) | [Jump](scene-serializer.md#sceneserializer-save) |
| `SceneChangedEvent` | struct | Old/new scene names | [Jump](event-dispatcher.md#struct-scenechangedevent) |
| `SceneManager` | class | Owns active scene | [Jump](scene-manager.md#class-scenemanager) |
| `SceneManager::SceneManager(EventDispatcher&)` | constructor | Bind dispatcher | [Jump](scene-manager.md#scenemanager-scenemanager) |
| `Scene::sceneName` | field | Name | [Jump](scene.md#scene-scenename) |
| `Scene` | class | Named set of entities | [Jump](scene.md#class-scene) |
| `Scene::Scene(std::string)` | constructor | Create with name | [Jump](scene.md#scene-scene) |
| `SceneSerializer` | class | JSON load/save | [Jump](scene-serializer.md#class-sceneserializer) |
| `SceneManager::setActiveScene(std::unique_ptr<Scene>)` | method | Swap scene, dispatch event | [Jump](scene-manager.md#scenemanager-setactivescene) |
| `ColliderComponent::setEnabled(bool)` | method | Turn on/off | [Jump](collider-component.md#collidercomponent-setenabled) |
| `ColliderComponent::setLocalBounds(float, float, float, float)` | method | Change hitbox (validated) | [Jump](collider-component.md#collidercomponent-setlocalbounds) |
| `Component::setOwner(Entity&)` | private method | Set back-pointer | [Jump](component.md#component-setowner) |
| `shutdownPlatform()` | free function | Release everything | [Jump](platform.md#shutdownplatform) |
| `AudioService::SoundId` | type alias | Sound ID (0 = none) | [Jump](audio-service.md#audioservice-soundid) |
| `Sprite::sourceSet(std::optional<Bounds>)` | method | Set source crop | [Jump](sprite.md#sprite-sourceset) |
| `Sprite` | class | Draws a texture | [Jump](sprite.md#class-sprite) |
| `Sprite::Sprite(textureHandle)` | constructor | Store handle | [Jump](sprite.md#sprite-sprite) |
| `Sprite::src` | field | Source crop | [Jump](sprite.md#sprite-src) |
| `AudioService::stop()` | method | Silence | [Jump](audio-service.md#audioservice-stop) |
| `EventDispatcher::subscribe(Handler)` | method | Add listener | [Jump](event-dispatcher.md#eventdispatcher-subscribe) |
| `EventDispatcher::subscriptions` | field | Listener list | [Jump](event-dispatcher.md#eventdispatcher-subscriptions) |
| `textureHandle` | type alias | Texture ID (0 = none) | [Jump](render-types.md#rendertypes-texturehandle) |
| `Renderer::textures` | field | Owned textures (index = handle) | [Jump](renderer.md#renderer-textures) |
| `toSDLScancode(Key)` | free function | Key → SDL scancode | [Jump](input.md#tosdlscancode) |
| `Transform` | class | Position and size | [Jump](transform.md#class-transform) |
| `EventDispatcher::unsubscribe(ListenerId)` | method | Remove listener | [Jump](event-dispatcher.md#eventdispatcher-unsubscribe) |
| `AnimationComponent::update(float)` | method | Advance animation | [Jump](animation-component.md#animationcomponent-update) |
| `Component::update(float)` | virtual method | Logic hook | [Jump](component.md#component-update) |
| `Entity::update(float)` | method | Update all components | [Jump](entity.md#entity-update) |
| `updatePlayerMovement(Scene&, float)` | free function | W/A/S/D move the player | [Jump](game.md#updateplayermovement) |
| `Scene::update(float)` | method | Update all entities | [Jump](scene.md#scene-update) |
| `SceneManager::update(float)` | method | Update active scene | [Jump](scene-manager.md#scenemanager-update) |
| `validateWav(...) (audio_service.cpp)` | private function | Size/format/duration checks | [Jump](audio-service.md#internals) |
| `wasKeyPressed(Key)` | free function | Pressed this frame? | [Jump](input.md#waskeypressed) |
| `wasKeyReleased(Key)` | free function | Released this frame? | [Jump](input.md#waskeyreleased) |
| `Bounds::w` | field | Width | [Jump](render-types.md#bounds-w) |
| `ColliderComponent::width` | field | Hitbox width | [Jump](collider-component.md#collidercomponent-width) |
| `Transform::width` | field | Width | [Jump](transform.md#transform-width) |
| `window` | global | SDL window | [Jump](platform.md#platform-window) |
| `Bounds::x` | field | Left edge | [Jump](render-types.md#bounds-x) |
| `Transform::x` | field | Left edge | [Jump](transform.md#transform-x) |
| `Bounds::y` | field | Top edge | [Jump](render-types.md#bounds-y) |
| `Transform::y` | field | Top edge | [Jump](transform.md#transform-y) |

## Naming rules

- Use qualified names, for example `Scene::render(...)`, and include enough parameter types to distinguish overloads.
- Alphabetize by the name shown in source, ignoring scope punctuation where practical.
- Use an explicit, unique anchor on the destination page, such as `<a id="scene-render"></a>`.
- For a field, link to its row under the class's **Fields** section; for meaningful variables not owned by a class, link to the module's **State and types** section.
- If a private helper matters only inside its `.cpp`, it **still** belongs here.
