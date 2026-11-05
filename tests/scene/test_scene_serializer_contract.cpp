#include "animationComponent.h"
#include "asset_manager.h"
#include "colliderComponent.h"
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

// The proposed Collider schema is:
//   "collider": { "offsetX": 1, "offsetY": 2,
//                 "width": 10, "height": 20, "enabled": false }
// Confirm these key names against Phase 3 worksheet section 3B before merging.
std::string WithCollider(const std::string& collider) {
    return std::string(R"({"version":1,"name":"With collider","entities":[{
        "transform":{"x":5,"y":10,"width":32,"height":32},
        "sprite":{"texture":"red.bmp"},
        "collider":)") + collider + R"(}]})";
}

std::string BasicScene(const std::string& transform,
                       const std::string& sprite = R"({"texture":"red.bmp"})") {
    return std::string(R"({"version":1,"name":"Basic","entities":[{
        "transform":)") + transform + R"(,"sprite":)" + sprite + R"(}]})";
}

std::string ReadAll(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    std::ostringstream output;
    output << file.rdbuf();
    return output.str();
}

class SceneSerializerContractTest : public ::testing::Test {
  protected:
    SDL_Window* window = nullptr;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<AssetManager> assets;
    std::filesystem::path temporaryDirectory;
    SceneSerializer serializer;

    void SetUp() override {
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
        window = SDL_CreateWindow("Serializer contract tests", 64, 64, SDL_WINDOW_HIDDEN);
        ASSERT_NE(window, nullptr) << SDL_GetError();
        renderer = std::make_unique<Renderer>(window);

        std::random_device random;
        for (int attempt = 0; attempt < 100 && temporaryDirectory.empty(); ++attempt) {
            auto candidate = std::filesystem::temp_directory_path() /
                             ("phase3-serializer-contract-" + std::to_string(random()));
            if (std::filesystem::create_directory(candidate)) {
                temporaryDirectory = std::move(candidate);
            }
        }
        ASSERT_FALSE(temporaryDirectory.empty());

        auto surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>(
            SDL_CreateSurface(4, 4, SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
        ASSERT_NE(surface, nullptr) << SDL_GetError();
        ASSERT_TRUE(SDL_SaveBMP(surface.get(), (temporaryDirectory / "red.bmp").string().c_str()))
            << SDL_GetError();

        assets = std::make_unique<AssetManager>(*renderer, temporaryDirectory);
    }

    void TearDown() override {
        assets.reset();
        renderer.reset();
        if (window != nullptr) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
        SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        if (!temporaryDirectory.empty()) {
            std::error_code error;
            std::filesystem::remove_all(temporaryDirectory, error);
        }
    }

    std::filesystem::path WriteText(const std::string& filename, const std::string& content) {
        const auto path = temporaryDirectory / filename;
        std::ofstream file(path, std::ios::binary);
        EXPECT_TRUE(file.is_open());
        file << content;
        file.close();
        EXPECT_TRUE(file.good());
        return path;
    }

    void ExpectDiagnostic(const std::string& json, const std::string& expectedField,
                          const std::string& expectedContext = "entity 0") {
        const auto path = WriteText("invalid.json", json);
        try {
            (void)serializer.load(path, *assets);
            FAIL() << "Expected invalid scene to be rejected: " << json;
        } catch (const std::exception& error) {
            const std::string message = error.what();
            EXPECT_NE(message.find(path.string()), std::string::npos) << message;
            EXPECT_NE(message.find(expectedContext), std::string::npos) << message;
            EXPECT_NE(message.find(expectedField), std::string::npos) << message;
        }
    }
};

TEST_F(SceneSerializerContractTest, LoadsRealPhase3DemoThroughAssetManager) {
    // CMake runs discovered tests with WORKING_DIRECTORY == PROJECT_SOURCE_DIR.
    const auto assetRoot = std::filesystem::current_path() / "assets";
    const auto demo = assetRoot / "scenes" / "phase3-demo.json";
    ASSERT_TRUE(std::filesystem::exists(demo)) << demo;

    AssetManager realAssets(*renderer, assetRoot);
    auto scene = serializer.load(demo, realAssets);
    ASSERT_NE(scene, nullptr);
    EXPECT_EQ(scene->getSceneName(), "Phase 3 Demo");
    ASSERT_EQ(scene->getEntities().size(), 3u);

    const auto& entities = scene->getEntities();
    auto* sprite0 = entities[0]->getComponent<Sprite>();
    auto* sprite1 = entities[1]->getComponent<Sprite>();
    auto* animated = entities[2]->getComponent<AnimationComponent>();
    ASSERT_NE(sprite0, nullptr);
    ASSERT_NE(sprite1, nullptr);
    ASSERT_NE(animated, nullptr);
    EXPECT_EQ(sprite0->getTexture(), sprite1->getTexture());
    EXPECT_EQ(realAssets.getPath(sprite0->getTexture()), "test.bmp");
    EXPECT_EQ(animated->getFrames().size(), 4u);
    EXPECT_TRUE(animated->getRepeat());
}

TEST_F(SceneSerializerContractTest, RejectsMissingSpriteWithEntityContext) {
    const std::string json = R"({"version":1,"name":"Missing sprite","entities":[{
        "transform":{"x":0,"y":0,"width":20,"height":20}}]})";
    ExpectDiagnostic(json, "sprite");
}

TEST_F(SceneSerializerContractTest, MissingTextureErrorNamesEntityAndTextureField) {
    const std::string json = BasicScene(
        R"({"x":0,"y":0,"width":20,"height":20})", R"({"texture":"missing.bmp"})");
    ExpectDiagnostic(json, "texture");
}

TEST_F(SceneSerializerContractTest, RejectsNonFiniteTransformValue) {
    // A valid JSON number can overflow a float when converted to Transform::x.
    const std::string json = BasicScene(
        R"({"x":1e100,"y":0,"width":20,"height":20})");
    ExpectDiagnostic(json, "x");
}

TEST_F(SceneSerializerContractTest, RejectsAnimationFrameWithZeroWidth) {
    const std::string json = R"({"version":1,"name":"Bad frame","entities":[{
        "transform":{"x":0,"y":0,"width":20,"height":20},
        "sprite":{"texture":"red.bmp"},
        "animation":{"frames":[{"x":0,"y":0,"w":0,"h":4}],
                     "frameDuration":0.25,"loop":true}}]})";
    ExpectDiagnostic(json, "w");
}

TEST_F(SceneSerializerContractTest, LoadColliderWithOffsetsDimensionsAndDisabledState) {
    const auto json = WithCollider(
        R"({"offsetX":2,"offsetY":-3,"width":10,"height":12,"enabled":false})");
    auto scene = serializer.load(WriteText("collider.json", json), *assets);
    ASSERT_EQ(scene->getEntities().size(), 1u);
    auto* collider = scene->getEntities()[0]->getComponent<ColliderComponent>();
    ASSERT_NE(collider, nullptr);
    EXPECT_FLOAT_EQ(collider->getOffsetX(), 2.0f);
    EXPECT_FLOAT_EQ(collider->getOffsetY(), -3.0f);
    EXPECT_FLOAT_EQ(collider->getWidth(), 10.0f);
    EXPECT_FLOAT_EQ(collider->getHeight(), 12.0f);
    EXPECT_FALSE(collider->isEnabled());
}

TEST_F(SceneSerializerContractTest, ColliderSaveLoadSavePreservesMeaningfulValues) {
    auto scene = serializer.load(
        WriteText("source.json", WithCollider(
            R"({"offsetX":1.5,"offsetY":-2,"width":10,"height":20,"enabled":true})")),
        *assets);

    const auto first = temporaryDirectory / "first.json";
    const auto second = temporaryDirectory / "second.json";
    serializer.save(*scene, first, *assets);
    auto reloaded = serializer.load(first, *assets);
    serializer.save(*reloaded, second, *assets);

    const auto firstJson = nlohmann::json::parse(ReadAll(first));
    const auto secondJson = nlohmann::json::parse(ReadAll(second));
    EXPECT_EQ(firstJson, secondJson);
    ASSERT_TRUE(firstJson["entities"][0].contains("collider"));
    EXPECT_EQ(firstJson["entities"][0]["collider"]["offsetX"], 1.5);
    EXPECT_EQ(firstJson["entities"][0]["collider"]["enabled"], true);
    EXPECT_FALSE(firstJson["entities"][0].contains("id"));
}

TEST_F(SceneSerializerContractTest, RejectsColliderWithoutWidthWithContext) {
    ExpectDiagnostic(WithCollider(
        R"({"offsetX":0,"offsetY":0,"height":12,"enabled":true})"), "width");
}

TEST_F(SceneSerializerContractTest, RejectsColliderWithNonPositiveWidth) {
    ExpectDiagnostic(WithCollider(
        R"({"offsetX":0,"offsetY":0,"width":0,"height":12,"enabled":true})"), "width");
}

TEST_F(SceneSerializerContractTest, RejectsColliderEnabledWhenNotBoolean) {
    ExpectDiagnostic(WithCollider(
        R"({"offsetX":0,"offsetY":0,"width":10,"height":12,"enabled":"yes"})"), "enabled");
}

TEST_F(SceneSerializerContractTest, SaveCanReplaceExistingValidFile) {
    const auto input = WriteText("source.json", BasicScene(
        R"({"x":1,"y":2,"width":20,"height":30})"));
    auto scene = serializer.load(input, *assets);
    const auto destination = WriteText("existing.json", "old content");

    ASSERT_NO_THROW(serializer.save(*scene, destination, *assets));
    EXPECT_EQ(nlohmann::json::parse(ReadAll(destination))["name"], "Basic");
    EXPECT_FALSE(std::filesystem::exists(destination.string() + ".tmp"));
}

TEST_F(SceneSerializerContractTest, FailedReplacementPreservesDestinationDirectory) {
    const auto input = WriteText("source.json", BasicScene(
        R"({"x":1,"y":2,"width":20,"height":30})"));
    auto scene = serializer.load(input, *assets);
    const auto destination = temporaryDirectory / "existing-directory";
    ASSERT_TRUE(std::filesystem::create_directory(destination));

    EXPECT_THROW(serializer.save(*scene, destination, *assets), std::runtime_error);
    EXPECT_TRUE(std::filesystem::is_directory(destination));
    EXPECT_FALSE(std::filesystem::exists(destination.string() + ".tmp"));
}

TEST_F(SceneSerializerContractTest, FailedColliderLoadDoesNotReplaceActiveSceneOrEmitEvent) {
    EventDispatcher dispatcher;
    SceneManager manager(dispatcher);
    manager.setActiveScene(std::make_unique<Scene>("Old scene"));

    int events = 0;
    dispatcher.subscribe([&](const SceneChangedEvent&) { ++events; });
    const auto invalid = WriteText("invalid-collider.json", WithCollider(
        R"({"offsetX":0,"offsetY":0,"width":-1,"height":12,"enabled":true})"));

    EXPECT_THROW(manager.setActiveScene(serializer.load(invalid, *assets)), std::exception);
    EXPECT_EQ(events, 0);
    EXPECT_EQ(manager.getActiveScene().getSceneName(), "Old scene");
}

} // namespace
