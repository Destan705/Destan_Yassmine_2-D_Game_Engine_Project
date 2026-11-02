
#include "component.h"
#include "entity.h"
#include "event_dispatcher.h"
#include "renderer.h"
#include "scene.h"
#include "scene_manager.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Records when an Entity is updated or rendered.
class TrackingComponent : public Component {
  public:
    TrackingComponent(std::vector<int>& calls, int id) : calls(calls), id(id) {
    }

    void update(float dt) override {
        calls.push_back(id);
        lastDeltaTime = dt;
    }

    void render(Renderer&) override {
        calls.push_back(id);
    }

    float lastDeltaTime = 0.0f;

  private:
    std::vector<int>& calls;
    int id;
};

// Lets tests detect when an Entity's components are destroyed.
class DestructionTracker : public Component {
  public:
    explicit DestructionTracker(bool& destroyed) : destroyed(destroyed) {
    }

    ~DestructionTracker() override {
        destroyed = true;
    }

  private:
    bool& destroyed;
};

} // namespace

// ============================================================
// Scene tests
// ============================================================

TEST(SceneTest, StoresSceneName) {
    Scene scene("MainMenu");

    EXPECT_EQ(scene.getSceneName(), "MainMenu");
}

TEST(SceneTest, CreatesEntitiesWithUniqueIncreasingIds) {
    Scene scene("Level1");

    Entity& first = scene.createEntity();
    Entity& second = scene.createEntity();
    Entity& third = scene.createEntity();

    EXPECT_EQ(first.getId(), 1u);
    EXPECT_EQ(second.getId(), 2u);
    EXPECT_EQ(third.getId(), 3u);
}

TEST(SceneTest, EntityIdsAreIndependentBetweenScenes) {
    Scene firstScene("Level1");
    Scene secondScene("Level2");

    EXPECT_EQ(firstScene.createEntity().getId(), 1u);
    EXPECT_EQ(firstScene.createEntity().getId(), 2u);
    EXPECT_EQ(secondScene.createEntity().getId(), 1u);
}

TEST(SceneTest, CreatesDistinctEntities) {
    Scene scene("Level1");

    Entity& first = scene.createEntity();
    Entity& second = scene.createEntity();

    EXPECT_NE(&first, &second);
}

TEST(SceneTest, UpdatesEntitiesInInsertionOrder) {
    Scene scene("Level1");
    std::vector<int> calls;

    scene.createEntity().addComponent<TrackingComponent>(calls, 1);
    scene.createEntity().addComponent<TrackingComponent>(calls, 2);
    scene.createEntity().addComponent<TrackingComponent>(calls, 3);

    scene.update(0.1f);

    EXPECT_EQ(calls, (std::vector<int>{1, 2, 3}));
}

TEST(SceneTest, PassesDeltaTimeToComponents) {
    Scene scene("Level1");
    std::vector<int> calls;

    auto& component = scene.createEntity().addComponent<TrackingComponent>(calls, 1);

    scene.update(0.25f);

    EXPECT_FLOAT_EQ(component.lastDeltaTime, 0.25f);
}

TEST(SceneTest, EmptySceneCanUpdate) {
    Scene scene("Empty");

    EXPECT_NO_THROW(scene.update(0.1f));
}

TEST(SceneTest, DestroysOwnedEntities) {
    bool destroyed = false;

    {
        Scene scene("Temporary");
        scene.createEntity().addComponent<DestructionTracker>(destroyed);

        EXPECT_FALSE(destroyed);
    }

    EXPECT_TRUE(destroyed);
}

// ============================================================
// SceneManager tests
// ============================================================

TEST(SceneManagerTest, ThrowsWhenNoActiveSceneExists) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    EXPECT_THROW(manager.getActiveScene(), std::runtime_error);
}

TEST(SceneManagerTest, StoresActiveScene) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    manager.setActiveScene(std::make_unique<Scene>("Level1"));

    EXPECT_EQ(manager.getActiveScene().getSceneName(), "Level1");
}

TEST(SceneManagerTest, ReplacesActiveScene) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    manager.setActiveScene(std::make_unique<Scene>("Level1"));
    manager.setActiveScene(std::make_unique<Scene>("Level2"));

    EXPECT_EQ(manager.getActiveScene().getSceneName(), "Level2");
}

TEST(SceneManagerTest, DestroysPreviousSceneOnReplacement) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    bool destroyed = false;

    auto first = std::make_unique<Scene>("Level1");
    first->createEntity().addComponent<DestructionTracker>(destroyed);

    manager.setActiveScene(std::move(first));

    EXPECT_FALSE(destroyed);

    manager.setActiveScene(std::make_unique<Scene>("Level2"));

    EXPECT_TRUE(destroyed);
}

TEST(SceneManagerTest, UpdatesOnlyActiveScene) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    std::vector<int> calls;

    auto first = std::make_unique<Scene>("Level1");
    first->createEntity().addComponent<TrackingComponent>(calls, 1);

    manager.setActiveScene(std::move(first));
    manager.update(0.1f);

    auto second = std::make_unique<Scene>("Level2");
    second->createEntity().addComponent<TrackingComponent>(calls, 2);

    manager.setActiveScene(std::move(second));
    manager.update(0.1f);

    EXPECT_EQ(calls, (std::vector<int>{1, 2}));
}

TEST(SceneManagerTest, UpdateWithoutActiveSceneDoesNothing) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    EXPECT_NO_THROW(manager.update(0.1f));
}

TEST(SceneManagerTest, RejectsNullScene) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    EXPECT_THROW(manager.setActiveScene(nullptr), std::invalid_argument);
}

TEST(SceneManagerTest, NullSceneDoesNotReplaceExistingScene) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    manager.setActiveScene(std::make_unique<Scene>("Level1"));

    EXPECT_THROW(manager.setActiveScene(nullptr), std::invalid_argument);

    EXPECT_EQ(manager.getActiveScene().getSceneName(), "Level1");
}

TEST(SceneManagerTest, FirstSceneDispatchesCorrectEvent) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    std::vector<SceneChangedEvent> events;

    dispatcher.subscribe([&](const SceneChangedEvent& event) { events.push_back(event); });

    manager.setActiveScene(std::make_unique<Scene>("Level1"));

    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].oldSceneName, "");
    EXPECT_EQ(events[0].newSceneName, "Level1");
}

TEST(SceneManagerTest, ReplacementDispatchesCorrectEvent) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    std::vector<SceneChangedEvent> events;

    dispatcher.subscribe([&](const SceneChangedEvent& event) { events.push_back(event); });

    manager.setActiveScene(std::make_unique<Scene>("Level1"));
    manager.setActiveScene(std::make_unique<Scene>("Level2"));

    ASSERT_EQ(events.size(), 2u);

    EXPECT_EQ(events[1].oldSceneName, "Level1");
    EXPECT_EQ(events[1].newSceneName, "Level2");
}

TEST(SceneManagerTest, SceneChangeEventIsSynchronous) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    bool received = false;

    dispatcher.subscribe([&](const SceneChangedEvent&) { received = true; });

    manager.setActiveScene(std::make_unique<Scene>("Level1"));

    EXPECT_TRUE(received);
}

TEST(SceneManagerTest, OldSceneDestroyedBeforeEvent) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    bool destroyed = false;
    bool destroyedWhenNotified = false;

    auto first = std::make_unique<Scene>("Level1");
    first->createEntity().addComponent<DestructionTracker>(destroyed);

    manager.setActiveScene(std::move(first));

    dispatcher.subscribe([&](const SceneChangedEvent&) { destroyedWhenNotified = destroyed; });

    manager.setActiveScene(std::make_unique<Scene>("Level2"));

    EXPECT_TRUE(destroyedWhenNotified);
}

TEST(SceneManagerTest, NewSceneActiveDuringEvent) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    std::string observedName;

    dispatcher.subscribe(
        [&](const SceneChangedEvent&) { observedName = manager.getActiveScene().getSceneName(); });

    manager.setActiveScene(std::make_unique<Scene>("Level2"));

    EXPECT_EQ(observedName, "Level2");
}

TEST(SceneManagerTest, EventNamesRemainValidAfterReplacement) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    std::vector<SceneChangedEvent> events;

    dispatcher.subscribe([&](const SceneChangedEvent& event) { events.push_back(event); });

    manager.setActiveScene(std::make_unique<Scene>("Level1"));
    manager.setActiveScene(std::make_unique<Scene>("Level2"));

    ASSERT_EQ(events.size(), 2u);

    EXPECT_EQ(events[1].oldSceneName, "Level1");
    EXPECT_EQ(events[1].newSceneName, "Level2");
}

TEST(SceneManagerTest, NullSceneDoesNotDispatchEvent) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);

    int eventCount = 0;

    dispatcher.subscribe([&](const SceneChangedEvent&) { ++eventCount; });

    manager.setActiveScene(std::make_unique<Scene>("Level1"));

    EXPECT_THROW(manager.setActiveScene(nullptr), std::invalid_argument);

    EXPECT_EQ(eventCount, 1);
}

TEST(SceneManagerTest, DestroysActiveSceneOnShutdown) {
    bool destroyed = false;

    {
        EventDispatcher dispatcher;
        SceneManager manager(dispatcher);

        auto scene = std::make_unique<Scene>("Level1");
        scene->createEntity().addComponent<DestructionTracker>(destroyed);

        manager.setActiveScene(std::move(scene));

        EXPECT_FALSE(destroyed);
    }

    EXPECT_TRUE(destroyed);
}

TEST(SceneTest, RendersEntitiesInInsertionOrder) {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window* window = SDL_CreateWindow("Test", 320, 240, SDL_WINDOW_HIDDEN);

    ASSERT_NE(window, nullptr);

    {
        Renderer renderer(window);
        Scene scene("Level1");
        std::vector<int> calls;

        scene.createEntity().addComponent<TrackingComponent>(calls, 1);
        scene.createEntity().addComponent<TrackingComponent>(calls, 2);
        scene.createEntity().addComponent<TrackingComponent>(calls, 3);

        scene.render(renderer);

        EXPECT_EQ(calls, (std::vector<int>{1, 2, 3}));
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
}
