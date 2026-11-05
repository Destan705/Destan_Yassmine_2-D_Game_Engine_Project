#include "animationComponent.h"
#include "asset_manager.h"
#include "event_dispatcher.h"
#include "renderer.h"
#include "scene.h"
#include "scene_manager.h"
#include "scene_serializer.h"
#include "sprite.h"
#include "transform.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {

// A valid version-1 scene: two entities share one texture, the third is animated.
const char* const kValidScene = R"({
    "version": 1,
    "name": "Test Scene",
    "entities": [
        {
            "transform": { "x": 100, "y": 150, "width": 50, "height": 50 },
            "sprite": { "texture": "red.bmp" }
        },
        {
            "transform": { "x": 200, "y": 300, "width": 40, "height": 60 },
            "sprite": { "texture": "red.bmp" }
        },
        {
            "transform": { "x": 300, "y": 150, "width": 32, "height": 32 },
            "sprite": { "texture": "sheet.bmp" },
            "animation": {
                "frames": [
                    { "x": 0,  "y": 0, "w": 32, "h": 32 },
                    { "x": 32, "y": 0, "w": 32, "h": 32 }
                ],
                "frameDuration": 0.25,
                "loop": false
            }
        }
    ]
})";

// Sets up SDL, a Renderer and an AssetManager whose asset root is a fresh temp folder.
class SceneSerializerTest : public ::testing::Test {
  protected:
    SDL_Window* window = nullptr;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<AssetManager> assets;
    std::filesystem::path tempDir;
    SceneSerializer serializer;

    void SetUp() override {
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
        window = SDL_CreateWindow("Serializer tests", 64, 64, SDL_WINDOW_HIDDEN);
        ASSERT_NE(window, nullptr) << SDL_GetError();
        renderer = std::make_unique<Renderer>(window);

        std::random_device random;
        for (int attempt = 0; attempt < 100 && tempDir.empty(); ++attempt) {
            auto candidate = std::filesystem::temp_directory_path() /
                             ("scene-serializer-test-" + std::to_string(random()));
            if (std::filesystem::create_directory(candidate)) {
                tempDir = candidate;
            }
        }
        ASSERT_FALSE(tempDir.empty()) << "Could not create a temporary test directory";

        CreateBmp("red.bmp", 4, 4);
        CreateBmp("sheet.bmp", 64, 32);
        assets = std::make_unique<AssetManager>(*renderer, tempDir);
    }

    void TearDown() override {
        assets.reset();
        renderer.reset(); // textures are freed before the window goes away
        if (window != nullptr) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
        SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        if (!tempDir.empty()) {
            std::error_code error;
            std::filesystem::remove_all(tempDir, error);
        }
    }

    void CreateBmp(const std::string& name, int width, int height) {
        SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        ASSERT_NE(surface, nullptr) << SDL_GetError();
        const auto path = (tempDir / name).string();
        const bool saved = SDL_SaveBMP(surface, path.c_str());
        SDL_DestroySurface(surface);
        ASSERT_TRUE(saved) << SDL_GetError();
    }

    std::filesystem::path WriteFile(const std::string& name, const std::string& text) {
        const auto path = tempDir / name;
        std::ofstream file(path, std::ios::binary);
        file << text;
        return path;
    }

    static std::string ReadFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Loads `text` and expects a runtime_error whose message contains `expected`.
    void ExpectLoadError(const std::string& text, const std::string& expected) {
        const auto path = WriteFile("bad.json", text);
        try {
            serializer.load(path, *assets);
            FAIL() << "Expected load to throw for: " << text;
        } catch (const std::runtime_error& error) {
            const std::string message = error.what();
            EXPECT_NE(message.find(expected), std::string::npos) << "Message was: " << message;
            EXPECT_NE(message.find(path.string()), std::string::npos)
                << "Message should name the file. Message was: " << message;
        }
    }
};

// Replaces one value in the valid scene, so each invalid test changes exactly one thing.
std::string ValidSceneWith(const std::string& from, const std::string& to) {
    std::string text = kValidScene;
    const auto position = text.find(from);
    EXPECT_NE(position, std::string::npos) << "Test setup: '" << from << "' not found";
    if (position != std::string::npos) {
        text.replace(position, from.size(), to);
    }
    return text;
}

// ============================================================
// Loading valid files
// ============================================================

TEST_F(SceneSerializerTest, LoadsSceneNameAndEntityOrder) {
    const auto path = WriteFile("scene.json", kValidScene);
    auto scene = serializer.load(path, *assets);

    ASSERT_NE(scene, nullptr);
    EXPECT_EQ(scene->getSceneName(), "Test Scene");
    ASSERT_EQ(scene->getEntities().size(), 3u);

    // Scene assigns fresh IDs in file order.
    EXPECT_EQ(scene->getEntities()[0]->getId(), 1u);
    EXPECT_EQ(scene->getEntities()[1]->getId(), 2u);
    EXPECT_EQ(scene->getEntities()[2]->getId(), 3u);
}

TEST_F(SceneSerializerTest, LoadsTransformValues) {
    auto scene = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    ASSERT_EQ(scene->getEntities().size(), 3u);

    Transform* transform = scene->getEntities()[1]->getComponent<Transform>();
    ASSERT_NE(transform, nullptr);
    EXPECT_FLOAT_EQ(transform->x, 200.0f);
    EXPECT_FLOAT_EQ(transform->y, 300.0f);
    EXPECT_FLOAT_EQ(transform->width, 40.0f);
    EXPECT_FLOAT_EQ(transform->height, 60.0f);
}

TEST_F(SceneSerializerTest, SpritePathsResolveThroughAssetManager) {
    auto scene = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    ASSERT_EQ(scene->getEntities().size(), 3u);

    Sprite* first = scene->getEntities()[0]->getComponent<Sprite>();
    Sprite* third = scene->getEntities()[2]->getComponent<Sprite>();
    ASSERT_NE(first, nullptr);
    ASSERT_NE(third, nullptr);
    EXPECT_EQ(assets->getPath(first->getTexture()), "red.bmp");
    EXPECT_EQ(assets->getPath(third->getTexture()), "sheet.bmp");
}

TEST_F(SceneSerializerTest, SharedTexturePathReusesOneHandle) {
    auto scene = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    ASSERT_EQ(scene->getEntities().size(), 3u);

    Sprite* first = scene->getEntities()[0]->getComponent<Sprite>();
    Sprite* second = scene->getEntities()[1]->getComponent<Sprite>();
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(first->getTexture(), second->getTexture());
}

TEST_F(SceneSerializerTest, LoadsAnimationOnlyWhereConfigured) {
    auto scene = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    ASSERT_EQ(scene->getEntities().size(), 3u);

    EXPECT_EQ(scene->getEntities()[0]->getComponent<AnimationComponent>(), nullptr);

    auto* animation = scene->getEntities()[2]->getComponent<AnimationComponent>();
    ASSERT_NE(animation, nullptr);
    EXPECT_FLOAT_EQ(animation->getDuration(), 0.25f);
    EXPECT_FALSE(animation->getRepeat());
    ASSERT_EQ(animation->getFrames().size(), 2u);
    EXPECT_FLOAT_EQ(animation->getFrames()[1].x, 32.0f);
    EXPECT_FLOAT_EQ(animation->getFrames()[1].w, 32.0f);
}

TEST_F(SceneSerializerTest, LoadsSceneWithNoEntities) {
    auto scene = serializer.load(
        WriteFile("empty.json", R"({"version": 1, "name": "Empty", "entities": []})"), *assets);
    ASSERT_NE(scene, nullptr);
    EXPECT_EQ(scene->getSceneName(), "Empty");
    EXPECT_TRUE(scene->getEntities().empty());
}

// ============================================================
// Rejecting invalid files
// ============================================================

TEST_F(SceneSerializerTest, MissingFileThrows) {
    EXPECT_THROW(serializer.load(tempDir / "does-not-exist.json", *assets), std::runtime_error);
}

TEST_F(SceneSerializerTest, BadSyntaxThrows) {
    ExpectLoadError(R"({"version": 1, "name": "Broken", "entities": [ )", "syntax");
}

TEST_F(SceneSerializerTest, DuplicateTopLevelKeyThrows) {
    ExpectLoadError(R"({"version": 1, "name": "A", "name": "B", "entities": []})",
                    "duplicate key 'name'");
}

TEST_F(SceneSerializerTest, DuplicateNestedKeyThrows) {
    ExpectLoadError(ValidSceneWith(R"("x": 100,)", R"("x": 100, "x": 999,)"), "duplicate key 'x'");
}

TEST_F(SceneSerializerTest, SameKeyInDifferentObjectsIsAllowed) {
    // Every entity has its own "transform" and "x"; that must not count as a duplicate.
    EXPECT_NO_THROW(serializer.load(WriteFile("scene.json", kValidScene), *assets));
}

TEST_F(SceneSerializerTest, UnsupportedVersionThrows) {
    ExpectLoadError(ValidSceneWith(R"("version": 1)", R"("version": 2)"), "version");
}

TEST_F(SceneSerializerTest, MissingVersionThrows) {
    ExpectLoadError(R"({"name": "No version", "entities": []})", "version");
}

TEST_F(SceneSerializerTest, VersionAsTextThrows) {
    ExpectLoadError(ValidSceneWith(R"("version": 1)", R"("version": "1")"), "version");
}

TEST_F(SceneSerializerTest, UnknownTopLevelFieldThrows) {
    ExpectLoadError(ValidSceneWith(R"("version": 1,)", R"("version": 1, "author": "me",)"),
                    "unknown field 'author'");
}

TEST_F(SceneSerializerTest, UnknownComponentThrows) {
    ExpectLoadError(ValidSceneWith(R"("sprite": { "texture": "sheet.bmp" },)",
                                   R"("sprite": { "texture": "sheet.bmp" }, "sound": {},)"),
                    "unknown field 'sound'");
}

TEST_F(SceneSerializerTest, UnknownTransformFieldReportsEntity) {
    ExpectLoadError(ValidSceneWith(R"("width": 40)", R"("widht": 40)"), "entity 1");
}

TEST_F(SceneSerializerTest, MissingTransformThrows) {
    ExpectLoadError(R"({"version": 1, "name": "S", "entities": [
                        { "sprite": { "texture": "red.bmp" } } ]})",
                    "'transform' is missing");
}

TEST_F(SceneSerializerTest, MissingRequiredTransformFieldThrows) {
    ExpectLoadError(ValidSceneWith(R"("width": 40, )", ""), "'width' is missing");
}

TEST_F(SceneSerializerTest, WrongValueTypeThrows) {
    ExpectLoadError(ValidSceneWith(R"("x": 200)", R"("x": "200")"),
                    "'x' is missing or not a number");
}

TEST_F(SceneSerializerTest, NonPositiveFrameDurationThrows) {
    ExpectLoadError(ValidSceneWith(R"("frameDuration": 0.25)", R"("frameDuration": 0)"),
                    "frameDuration");
}

TEST_F(SceneSerializerTest, EmptyAnimationFramesThrow) {
    std::string text = kValidScene;
    const auto start = text.find(R"("frames": [)");
    const auto end = text.find(']', start);
    ASSERT_NE(start, std::string::npos);
    text.replace(start, end - start + 1, R"("frames": [])");
    ExpectLoadError(text, "frames");
}

TEST_F(SceneSerializerTest, AnimationLoopMustBeTrueOrFalse) {
    ExpectLoadError(ValidSceneWith(R"("loop": false)", R"("loop": "no")"), "loop");
}

TEST_F(SceneSerializerTest, MissingTextureFileThrows) {
    const auto path = WriteFile(
        "scene.json", ValidSceneWith(R"("texture": "sheet.bmp")", R"("texture": "missing.bmp")"));
    EXPECT_THROW(serializer.load(path, *assets), std::runtime_error);
}

// ============================================================
// Saving
// ============================================================

TEST_F(SceneSerializerTest, SaveLoadSavePreservesMeaningfulValues) {
    auto original = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    ASSERT_EQ(original->getEntities().size(), 3u);

    const auto firstSave = tempDir / "first.json";
    serializer.save(*original, firstSave, *assets);

    auto reloaded = serializer.load(firstSave, *assets);
    const auto secondSave = tempDir / "second.json";
    serializer.save(*reloaded, secondSave, *assets);

    // Compare parsed data, not text, so whitespace and key order don't matter.
    const auto first = nlohmann::json::parse(ReadFile(firstSave));
    const auto second = nlohmann::json::parse(ReadFile(secondSave));
    EXPECT_EQ(first, second);

    // And the saved data matches the original file's meaning.
    EXPECT_EQ(first, nlohmann::json::parse(kValidScene));
}

TEST_F(SceneSerializerTest, SavedFileContainsNoRuntimeState) {
    auto scene = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    const auto saved = tempDir / "saved.json";
    serializer.save(*scene, saved, *assets);

    const std::string text = ReadFile(saved);
    for (const char* forbidden : {"\"id\"", "handle", "elapsed", "currentFrame", "paused"}) {
        EXPECT_EQ(text.find(forbidden), std::string::npos) << "Saved file contains " << forbidden;
    }
}

TEST_F(SceneSerializerTest, SaveLeavesNoTemporaryFile) {
    auto scene = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    const auto saved = tempDir / "saved.json";
    serializer.save(*scene, saved, *assets);

    EXPECT_TRUE(std::filesystem::exists(saved));
    EXPECT_FALSE(std::filesystem::exists(tempDir / "saved.json.tmp"));
}

TEST_F(SceneSerializerTest, FailedSaveKeepsPreviousFile) {
    auto scene = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    const auto destination = WriteFile("keep.json", "previous good content");

    // A folder where the temp file should go makes opening the temp file fail.
    std::filesystem::create_directory(tempDir / "keep.json.tmp");

    EXPECT_THROW(serializer.save(*scene, destination, *assets), std::runtime_error);
    EXPECT_EQ(ReadFile(destination), "previous good content");
}

TEST_F(SceneSerializerTest, SavingUnsupportedEntityFailsAndKeepsPreviousFile) {
    Scene scene("Incomplete");
    scene.createEntity().addComponent<Transform>(); // no Sprite
    const auto destination = WriteFile("keep.json", "previous good content");

    EXPECT_THROW(serializer.save(scene, destination, *assets), std::runtime_error);
    EXPECT_EQ(ReadFile(destination), "previous good content");
}

// ============================================================
// Activation through SceneManager
// ============================================================

TEST_F(SceneSerializerTest, SuccessfulLoadThenActivateEmitsOneEvent) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);
    std::vector<SceneChangedEvent> events;
    dispatcher.subscribe([&](const SceneChangedEvent& event) { events.push_back(event); });

    auto scene = serializer.load(WriteFile("scene.json", kValidScene), *assets);
    EXPECT_TRUE(events.empty()) << "The serializer itself must not dispatch events";

    manager.setActiveScene(std::move(scene));
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events[0].newSceneName, "Test Scene");
}

TEST_F(SceneSerializerTest, FailedLoadKeepsOldSceneAndEmitsNoEvent) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);
    manager.setActiveScene(std::make_unique<Scene>("Old Scene"));

    int eventCount = 0;
    dispatcher.subscribe([&](const SceneChangedEvent&) { ++eventCount; });

    const auto bad = WriteFile("bad.json", ValidSceneWith(R"("version": 1)", R"("version": 9)"));
    EXPECT_THROW(manager.setActiveScene(serializer.load(bad, *assets)), std::runtime_error);

    EXPECT_EQ(eventCount, 0);
    EXPECT_EQ(manager.getActiveScene().getSceneName(), "Old Scene");
}

} // namespace
