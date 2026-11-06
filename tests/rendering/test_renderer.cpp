#include "renderer.h"
#include "scene.h"

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>

// A renderer owns an SDL resource, so copying it must stay disabled.
static_assert(!std::is_copy_constructible_v<Renderer>);
static_assert(!std::is_copy_assignable_v<Renderer>);

namespace {

using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;

class RendererTest : public ::testing::Test {
  protected:
    SDL_Window* window = nullptr;

    void SetUp() override {
        // Software rendering makes pixel comparisons independent of the GPU.
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
        window = SDL_CreateWindow("Renderer tests", 64, 64, SDL_WINDOW_HIDDEN);
        ASSERT_NE(window, nullptr) << SDL_GetError();
    }

    void TearDown() override {
        // Local Renderer objects are destroyed before this releases the window.
        SDL_DestroyWindow(window);
        SDL_Quit();
        SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
    }

    SurfacePtr ReadPixels() {
        // Read before Present(): the backbuffer is not preserved after presenting.
        return SurfacePtr(SDL_RenderReadPixels(SDL_GetRenderer(window), nullptr),
                          SDL_DestroySurface);
    }

    void ExpectPixel(SDL_Surface* surface, int x, int y, const Color& expected) {
        SCOPED_TRACE(::testing::Message() << "Pixel (" << x << ", " << y << ")");
        Uint8 red = 0;
        Uint8 green = 0;
        Uint8 blue = 0;
        Uint8 alpha = 0;
        ASSERT_TRUE(SDL_ReadSurfacePixel(surface, x, y, &red, &green, &blue, &alpha))
            << SDL_GetError();
        EXPECT_EQ(red, expected.red);
        EXPECT_EQ(green, expected.green);
        EXPECT_EQ(blue, expected.blue);
        EXPECT_EQ(alpha, expected.alpha);
    }
};

TEST_F(RendererTest, CreatesRendererForValidWindow) {
    EXPECT_EQ(SDL_GetRenderer(window), nullptr);
    ASSERT_NO_THROW({
        Renderer renderer(window);
        EXPECT_NE(SDL_GetRenderer(window), nullptr);
    });
}

TEST_F(RendererTest, RejectsNullWindow) {
    EXPECT_THROW(Renderer renderer(nullptr), std::runtime_error);
}

TEST_F(RendererTest, ReleasesRendererWhenItGoesOutOfScope) {
    {
        Renderer renderer(window);
        ASSERT_NE(SDL_GetRenderer(window), nullptr);
    }
    EXPECT_EQ(SDL_GetRenderer(window), nullptr);

    // The window remains usable, and another renderer can be created for it.
    EXPECT_NO_THROW(Renderer replacement(window));
}

TEST_F(RendererTest, ClearScreenFillsEntireDrawingAreaWithRequestedColor) {
    Renderer renderer(window);
    const Color background{17, 83, 149, 255};
    renderer.ClearScreen(background);

    auto pixels = ReadPixels();
    ASSERT_NE(pixels, nullptr) << SDL_GetError();
    for (int y = 0; y < pixels->h; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            ExpectPixel(pixels.get(), x, y, background);
        }
    }
}

TEST_F(RendererTest, DrawFilledRectUsesRequestedColorPositionAndSize) {
    Renderer renderer(window);
    const Color background{0, 0, 0, 255};
    const Color rectangle{255, 0, 0, 255};
    renderer.ClearScreen(background);
    // Distinct coordinates and dimensions catch swapped x/y or width/height.
    renderer.DrawFilledRect(rectangle, Bounds{7.0f, 13.0f, 19.0f, 11.0f});

    auto pixels = ReadPixels();
    ASSERT_NE(pixels, nullptr) << SDL_GetError();
    for (int y = 0; y < pixels->h; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            const bool inside = x >= 7 && x < 26 && y >= 13 && y < 24;
            ExpectPixel(pixels.get(), x, y, inside ? rectangle : background);
        }
    }
}

TEST_F(RendererTest, ClearScreenRemovesPreviouslyDrawnContent) {
    Renderer renderer(window);
    renderer.ClearScreen(Color{0, 0, 0, 255});
    renderer.DrawFilledRect(Color{255, 0, 0, 255}, Bounds{7.0f, 13.0f, 19.0f, 11.0f});
    const Color nextBackground{31, 97, 163, 255};
    renderer.ClearScreen(nextBackground);

    auto pixels = ReadPixels();
    ASSERT_NE(pixels, nullptr) << SDL_GetError();
    for (int y = 0; y < pixels->h; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            ExpectPixel(pixels.get(), x, y, nextBackground);
        }
    }
}

TEST_F(RendererTest, LaterRectangleOverwritesOverlapAndPreservesOtherPixels) {
    Renderer renderer(window);
    const Color black{0, 0, 0, 255};
    const Color red{255, 0, 0, 255};
    const Color blue{0, 0, 255, 255};
    renderer.ClearScreen(black);
    renderer.DrawFilledRect(red, Bounds{5.0f, 8.0f, 20.0f, 16.0f});
    renderer.DrawFilledRect(blue, Bounds{15.0f, 18.0f, 22.0f, 12.0f});

    auto pixels = ReadPixels();
    ASSERT_NE(pixels, nullptr) << SDL_GetError();
    for (int y = 0; y < pixels->h; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            const bool inRed = x >= 5 && x < 25 && y >= 8 && y < 24;
            const bool inBlue = x >= 15 && x < 37 && y >= 18 && y < 30;
            const Color& expected = inBlue ? blue : (inRed ? red : black);
            ExpectPixel(pixels.get(), x, y, expected);
        }
    }
}

class TextureTest : public RendererTest {
  protected:
    std::filesystem::path temporaryDirectory;

    void SetUp() override {
        RendererTest::SetUp();
        if (HasFatalFailure()) {
            return;
        }

        // Create a unique folder so parallel tests don't share files.
        std::random_device random;
        const auto temporaryRoot = std::filesystem::temp_directory_path();

        for (int attempt = 0; attempt < 100; ++attempt) {
            const auto candidate =
                temporaryRoot / ("engine-texture-test-" + std::to_string(random()));

            if (std::filesystem::create_directory(candidate)) {
                temporaryDirectory = candidate;
                break;
            }
        }

        ASSERT_FALSE(temporaryDirectory.empty()) << "Could not create a temporary test directory";
    }

    void TearDown() override {
        RendererTest::TearDown();

        if (!temporaryDirectory.empty()) {
            std::error_code error;
            std::filesystem::remove_all(temporaryDirectory, error);
            EXPECT_FALSE(error) << error.message();
        }
    }

    std::string FilePath(const std::string& name) {
        // SDL expects UTF-8 paths, including on Windows.
        const auto utf8 = (temporaryDirectory / name).u8string();
        return std::string(utf8.begin(), utf8.end());
    }

    void CreateBmp(const std::string& path, const Color& color) {
        SurfacePtr surface(SDL_CreateSurface(2, 2, SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
        ASSERT_NE(surface, nullptr) << SDL_GetError();

        const Uint32 pixel = SDL_MapSurfaceRGB(surface.get(), color.red, color.green, color.blue);

        ASSERT_TRUE(SDL_FillSurfaceRect(surface.get(), nullptr, pixel)) << SDL_GetError();

        ASSERT_TRUE(SDL_SaveBMP(surface.get(), path.c_str())) << SDL_GetError();
    }

    void ExpectSolidScreen(const Color& expected) {
        auto pixels = ReadPixels();
        ASSERT_NE(pixels, nullptr) << SDL_GetError();

        for (int y = 0; y < pixels->h; ++y) {
            for (int x = 0; x < pixels->w; ++x) {
                ExpectPixel(pixels.get(), x, y, expected);
            }
        }
    }
};

TEST_F(TextureTest, LoadsValidBmpAndDrawsAtRequestedPositionAndSize) {
    const Color background{0, 0, 0, 255};
    const Color textureColor{255, 0, 0, 255};
    const std::string path = FilePath("red.bmp");

    ASSERT_NO_FATAL_FAILURE(CreateBmp(path, textureColor));

    Renderer renderer(window);
    textureHandle currentTexture = 0;
    ASSERT_NO_THROW(currentTexture = renderer.LoadTexture(path.c_str()));

    renderer.ClearScreen(background);
    renderer.DrawTexture(currentTexture, Bounds{7.0f, 13.0f, 19.0f, 11.0f});

    auto pixels = ReadPixels();
    ASSERT_NE(pixels, nullptr) << SDL_GetError();

    // Check the texture's area and the untouched background.
    for (int y = 0; y < pixels->h; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            const bool inside = x >= 7 && x < 26 && y >= 13 && y < 24;

            ExpectPixel(pixels.get(), x, y, inside ? textureColor : background);
        }
    }
}

TEST_F(TextureTest, MissingFileThrowsRuntimeError) {
    Renderer renderer(window);
    const std::string path = FilePath("missing.bmp");

    EXPECT_THROW(renderer.LoadTexture(path.c_str()), std::runtime_error);
}

TEST_F(TextureTest, InvalidBmpThrowsRuntimeError) {
    const auto path = temporaryDirectory / "invalid.bmp";

    // The extension says BMP, but the contents aren't an image.
    {
        std::ofstream file(path, std::ios::binary);
        ASSERT_TRUE(file.is_open());
        file << "This is not a BMP image";
        file.close();
        ASSERT_FALSE(file.fail());
    }

    Renderer renderer(window);
    const std::string texturePath = FilePath("invalid.bmp");

    EXPECT_THROW(renderer.LoadTexture(texturePath.c_str()), std::runtime_error);
}

TEST_F(TextureTest, DrawingWithoutTextureLeavesScreenUnchanged) {
    Renderer renderer(window);
    const Color background{17, 83, 149, 255};
    renderer.ClearScreen(background);

    EXPECT_NO_THROW(renderer.DrawTexture(0, Bounds{7.0f, 13.0f, 19.0f, 11.0f}));

    ExpectSolidScreen(background);
}

TEST_F(TextureTest, LoadedTexturesReceiveDistinctHandles) {
    const std::string redPath = FilePath("red.bmp");
    const std::string bluePath = FilePath("blue.bmp");

    ASSERT_NO_FATAL_FAILURE(CreateBmp(redPath, Color{255, 0, 0, 255}));
    ASSERT_NO_FATAL_FAILURE(CreateBmp(bluePath, Color{0, 0, 255, 255}));

    Renderer renderer(window);

    const textureHandle redHandle = renderer.LoadTexture(redPath.c_str());
    const textureHandle blueHandle = renderer.LoadTexture(bluePath.c_str());

    EXPECT_NE(redHandle, 0u);
    EXPECT_NE(blueHandle, 0u);
    EXPECT_NE(redHandle, blueHandle);
}

TEST_F(TextureTest, DrawsTwoDifferentTexturesInSameFrame) {
    const Color background{0, 0, 0, 255};
    const Color red{255, 0, 0, 255};
    const Color blue{0, 0, 255, 255};

    const std::string redPath = FilePath("red.bmp");
    const std::string bluePath = FilePath("blue.bmp");

    ASSERT_NO_FATAL_FAILURE(CreateBmp(redPath, red));
    ASSERT_NO_FATAL_FAILURE(CreateBmp(bluePath, blue));

    Renderer renderer(window);

    const textureHandle redHandle = renderer.LoadTexture(redPath.c_str());
    const textureHandle blueHandle = renderer.LoadTexture(bluePath.c_str());

    renderer.ClearScreen(background);

    renderer.DrawTexture(redHandle, Bounds{5, 10, 20, 20});
    renderer.DrawTexture(blueHandle, Bounds{35, 10, 20, 20});

    auto pixels = ReadPixels();
    ASSERT_NE(pixels, nullptr) << SDL_GetError();

    ExpectPixel(pixels.get(), 10, 15, red);
    ExpectPixel(pixels.get(), 40, 15, blue);
    ExpectPixel(pixels.get(), 30, 15, background);
}

TEST_F(TextureTest, InvalidHandleDoesNotChangeScreen) {
    Renderer renderer(window);

    const Color background{25, 50, 75, 255};
    renderer.ClearScreen(background);

    EXPECT_NO_THROW(renderer.DrawTexture(9999u, Bounds{0, 0, 64, 64}));

    ExpectSolidScreen(background);
}

TEST_F(TextureTest, LoadingSecondTexturePreservesFirstHandle) {
    const Color red{255, 0, 0, 255};
    const Color blue{0, 0, 255, 255};

    const std::string redPath = FilePath("red.bmp");
    const std::string bluePath = FilePath("blue.bmp");

    ASSERT_NO_FATAL_FAILURE(CreateBmp(redPath, red));
    ASSERT_NO_FATAL_FAILURE(CreateBmp(bluePath, blue));

    Renderer renderer(window);

    const textureHandle redHandle = renderer.LoadTexture(redPath.c_str());
    const textureHandle blueHandle = renderer.LoadTexture(bluePath.c_str());

    renderer.ClearScreen(Color{0, 0, 0, 255});
    renderer.DrawTexture(redHandle, Bounds{0, 0, 64, 64});

    ExpectSolidScreen(red);

    renderer.ClearScreen(Color{0, 0, 0, 255});
    renderer.DrawTexture(blueHandle, Bounds{0, 0, 64, 64});

    ExpectSolidScreen(blue);
}

TEST_F(TextureTest, DestroysRendererWithMultipleLoadedTextures) {
    const std::string redPath = FilePath("red.bmp");
    const std::string bluePath = FilePath("blue.bmp");

    ASSERT_NO_FATAL_FAILURE(CreateBmp(redPath, Color{255, 0, 0, 255}));
    ASSERT_NO_FATAL_FAILURE(CreateBmp(bluePath, Color{0, 0, 255, 255}));

    {
        Renderer renderer(window);

        renderer.LoadTexture(redPath.c_str());
        renderer.LoadTexture(bluePath.c_str());

        ASSERT_NE(SDL_GetRenderer(window), nullptr);
    }

    EXPECT_EQ(SDL_GetRenderer(window), nullptr);

    // A new renderer should still be creatable for the same window.
    EXPECT_NO_THROW(Renderer replacement(window));
}

} // namespace
