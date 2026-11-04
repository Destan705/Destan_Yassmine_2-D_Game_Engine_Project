#include "asset_manager.h"
#include "renderer.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {

// A real software Renderer plus an AssetManager rooted in a fresh temp folder.
class AssetManagerTest : public ::testing::Test {
  protected:
    SDL_Window* window = nullptr;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<AssetManager> assets;
    std::filesystem::path root;

    void SetUp() override {
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
        window = SDL_CreateWindow("AssetManager tests", 64, 64, SDL_WINDOW_HIDDEN);
        ASSERT_NE(window, nullptr) << SDL_GetError();
        renderer = std::make_unique<Renderer>(window);

        std::random_device random;
        for (int attempt = 0; attempt < 100 && root.empty(); ++attempt) {
            auto candidate = std::filesystem::temp_directory_path() /
                             ("asset-manager-test-" + std::to_string(random()));
            if (std::filesystem::create_directory(candidate)) {
                root = candidate;
            }
        }
        ASSERT_FALSE(root.empty()) << "Could not create a temporary asset folder";

        std::filesystem::create_directories(root / "sprites");
        CreateBmp("red.bmp", 4, 4);
        CreateBmp("sprites/cat.bmp", 16, 8);
        assets = std::make_unique<AssetManager>(*renderer, root);
    }

    void TearDown() override {
        assets.reset();
        renderer.reset();
        if (window != nullptr) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
        SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        if (!root.empty()) {
            std::error_code error;
            std::filesystem::remove_all(root, error);
        }
    }

    void CreateBmp(const std::string& relativePath, int width, int height) {
        SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        ASSERT_NE(surface, nullptr) << SDL_GetError();
        const auto path = (root / relativePath).string();
        const bool saved = SDL_SaveBMP(surface, path.c_str());
        SDL_DestroySurface(surface);
        ASSERT_TRUE(saved) << SDL_GetError();
    }
};

// ---------- Caching ----------

TEST_F(AssetManagerTest, LoadsTextureAndReturnsValidHandle) {
    EXPECT_NE(assets->loadTexture("red.bmp"), 0u);
}

TEST_F(AssetManagerTest, SamePathReturnsSameHandle) {
    const textureHandle first = assets->loadTexture("red.bmp");
    const textureHandle second = assets->loadTexture("red.bmp");
    EXPECT_EQ(first, second);
}

TEST_F(AssetManagerTest, RepeatedPathLoadsOnlyOnce) {
    const textureHandle first = assets->loadTexture("red.bmp");
    assets->loadTexture("red.bmp");
    assets->loadTexture("red.bmp");

    // If the cache worked, the Renderer created no extra textures, so the next
    // new file gets the very next handle.
    const textureHandle next = assets->loadTexture("sprites/cat.bmp");
    EXPECT_EQ(next, first + 1);
}

TEST_F(AssetManagerTest, EquivalentPathSpellingsShareOneHandle) {
    const textureHandle normal = assets->loadTexture("sprites/cat.bmp");
    EXPECT_EQ(assets->loadTexture("./sprites/cat.bmp"), normal);
    EXPECT_EQ(assets->loadTexture("sprites/./cat.bmp"), normal);
    EXPECT_EQ(assets->loadTexture("sprites\\cat.bmp"), normal);
    EXPECT_EQ(assets->loadTexture("sprites//cat.bmp"), normal);
}

TEST_F(AssetManagerTest, DifferentFilesGetDifferentHandles) {
    EXPECT_NE(assets->loadTexture("red.bmp"), assets->loadTexture("sprites/cat.bmp"));
}

// ---------- Reverse lookup ----------

TEST_F(AssetManagerTest, GetPathReturnsNormalizedPath) {
    const textureHandle handle = assets->loadTexture("./sprites\\cat.bmp");
    EXPECT_EQ(assets->getPath(handle), "sprites/cat.bmp");
}

TEST_F(AssetManagerTest, GetPathOfUnknownHandleThrows) {
    EXPECT_THROW(assets->getPath(0), std::out_of_range);
    EXPECT_THROW(assets->getPath(999), std::out_of_range);
}

// ---------- Failures and retry ----------

TEST_F(AssetManagerTest, MissingFileThrows) {
    EXPECT_THROW(assets->loadTexture("missing.bmp"), std::runtime_error);
}

TEST_F(AssetManagerTest, FailedLoadIsNotCachedAndCanBeRetried) {
    EXPECT_THROW(assets->loadTexture("later.bmp"), std::runtime_error);

    CreateBmp("later.bmp", 2, 2);
    textureHandle handle = 0;
    EXPECT_NO_THROW(handle = assets->loadTexture("later.bmp"));
    EXPECT_NE(handle, 0u);
    EXPECT_EQ(assets->getPath(handle), "later.bmp");
}

TEST_F(AssetManagerTest, FailedLoadDoesNotAffectExistingEntries) {
    const textureHandle red = assets->loadTexture("red.bmp");
    EXPECT_THROW(assets->loadTexture("missing.bmp"), std::runtime_error);
    EXPECT_EQ(assets->loadTexture("red.bmp"), red);
    EXPECT_EQ(assets->getPath(red), "red.bmp");
}

// ---------- Invalid paths ----------

TEST_F(AssetManagerTest, RejectsEmptyPath) {
    EXPECT_THROW(assets->loadTexture(""), std::invalid_argument);
    EXPECT_THROW(assets->loadTexture("."), std::invalid_argument);
}

TEST_F(AssetManagerTest, RejectsAbsoluteAndRootedPaths) {
    EXPECT_THROW(assets->loadTexture("/red.bmp"), std::invalid_argument);
    EXPECT_THROW(assets->loadTexture("//server/red.bmp"), std::invalid_argument);
    EXPECT_THROW(assets->loadTexture((root / "red.bmp").string()), std::invalid_argument);
}

TEST_F(AssetManagerTest, RejectsWindowsDrivePaths) {
    EXPECT_THROW(assets->loadTexture("C:/red.bmp"), std::invalid_argument);
    EXPECT_THROW(assets->loadTexture("c:red.bmp"), std::invalid_argument);
}

TEST_F(AssetManagerTest, RejectsParentTraversal) {
    EXPECT_THROW(assets->loadTexture("../red.bmp"), std::invalid_argument);
    EXPECT_THROW(assets->loadTexture("sprites/../red.bmp"), std::invalid_argument);
}

// ---------- Texture dimensions ----------

TEST_F(AssetManagerTest, ReportsOriginalTextureDimensions) {
    const textureHandle cat = assets->loadTexture("sprites/cat.bmp");
    const auto [width, height] = assets->getOriginalDimensions(cat);
    EXPECT_EQ(width, 16);
    EXPECT_EQ(height, 8);
}

TEST_F(AssetManagerTest, DimensionsOfUnknownHandleThrow) {
    EXPECT_THROW(assets->getOriginalDimensions(42), std::out_of_range);
}

TEST_F(AssetManagerTest, RendererDimensionQueryMatchesTexture) {
    const textureHandle red = assets->loadTexture("red.bmp");
    const auto [width, height] = renderer->getTextureDimensions(red);
    EXPECT_FLOAT_EQ(width, 4.0f);
    EXPECT_FLOAT_EQ(height, 4.0f);
}

TEST_F(AssetManagerTest, RendererDimensionQueryRejectsInvalidHandle) {
    EXPECT_THROW(renderer->getTextureDimensions(0), std::invalid_argument);
    EXPECT_THROW(renderer->getTextureDimensions(999), std::invalid_argument);
}

} // namespace
