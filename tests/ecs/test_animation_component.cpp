#include "animation_component.h"
#include "entity.h"
#include "renderer.h"
#include "sprite_component.h"
#include "transform_component.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <cmath>
#include <filesystem>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {

using SurfacePtr = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;

const std::vector<Bounds> kFrames = {
    {0.0f, 0.0f, 2.0f, 2.0f},
    {2.0f, 0.0f, 2.0f, 2.0f},
};

// These tests deliberately inspect rendered pixels. An AnimationComponent can
// update a Sprite's source rectangle correctly while the Renderer ignores it.
class AnimationComponentContractTest : public ::testing::Test {
  protected:
    SDL_Window* window = nullptr;
    std::filesystem::path temporaryDirectory;

    void SetUp() override {
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
        window = SDL_CreateWindow("Animation tests", 32, 32, SDL_WINDOW_HIDDEN);
        ASSERT_NE(window, nullptr) << SDL_GetError();

        std::random_device random;
        for (int attempt = 0; attempt < 100 && temporaryDirectory.empty(); ++attempt) {
            auto candidate = std::filesystem::temp_directory_path() /
                             ("phase3-animation-test-" + std::to_string(random()));
            if (std::filesystem::create_directory(candidate)) {
                temporaryDirectory = std::move(candidate);
            }
        }
        ASSERT_FALSE(temporaryDirectory.empty());
    }

    void TearDown() override {
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

    std::string CreateTwoFrameBmp() {
        SurfacePtr surface(SDL_CreateSurface(4, 2, SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
        if (surface == nullptr) {
            ADD_FAILURE() << SDL_GetError();
            return {};
        }

        const SDL_Rect left{0, 0, 2, 2};
        const SDL_Rect right{2, 0, 2, 2};
        const Uint32 red = SDL_MapSurfaceRGB(surface.get(), 255, 0, 0);
        const Uint32 green = SDL_MapSurfaceRGB(surface.get(), 0, 255, 0);

        if (!SDL_FillSurfaceRect(surface.get(), &left, red) ||
            !SDL_FillSurfaceRect(surface.get(), &right, green)) {
            ADD_FAILURE() << SDL_GetError();
            return {};
        }

        const auto path = temporaryDirectory / "two-frames.bmp";
        const std::string fileName = path.string();
        if (!SDL_SaveBMP(surface.get(), fileName.c_str())) {
            ADD_FAILURE() << SDL_GetError();
            return {};
        }
        return fileName;
    }

    Color SampleEntityColor(Renderer& renderer, Entity& entity) {
        renderer.ClearScreen(Color{0, 0, 0, 255});
        entity.render(renderer);

        SurfacePtr pixels(SDL_RenderReadPixels(SDL_GetRenderer(window), nullptr),
                          SDL_DestroySurface);
        if (pixels == nullptr) {
            ADD_FAILURE() << SDL_GetError();
            return Color{0, 0, 0, 0};
        }
        Uint8 red = 0;
        Uint8 green = 0;
        Uint8 blue = 0;
        Uint8 alpha = 0;
        if (!SDL_ReadSurfacePixel(pixels.get(), 8, 8, &red, &green, &blue, &alpha)) {
            ADD_FAILURE() << SDL_GetError();
            return Color{0, 0, 0, 0};
        }
        return Color{red, green, blue, alpha};
    }

    void ExpectColor(const Color& actual, Uint8 red, Uint8 green, Uint8 blue) {
        EXPECT_EQ(actual.red, red);
        EXPECT_EQ(actual.green, green);
        EXPECT_EQ(actual.blue, blue);
    }
};

TEST(AnimationComponentValidationTest, RejectsEmptyFrames) {
    EXPECT_THROW(AnimationComponent({}, 0.25f, true), std::invalid_argument);
}

TEST(AnimationComponentValidationTest, RejectsInvalidFrameDuration) {
    for (float duration : {0.0f, -1.0f, std::numeric_limits<float>::infinity(),
                           std::numeric_limits<float>::quiet_NaN()}) {
        EXPECT_THROW(AnimationComponent(kFrames, duration, true), std::invalid_argument);
    }
}

TEST(AnimationComponentValidationTest, ExposesStableConfigurationForSerialization) {
    AnimationComponent animation(kFrames, 0.25f, false);
    EXPECT_FLOAT_EQ(animation.getDuration(), 0.25f);
    EXPECT_FALSE(animation.getRepeat());
    ASSERT_EQ(animation.getFrames().size(), 2u);
    EXPECT_FLOAT_EQ(animation.getFrames()[1].x, 2.0f);
    EXPECT_FLOAT_EQ(animation.getFrames()[1].w, 2.0f);
}

TEST(AnimationComponentValidationTest, SafelyUpdatesWithoutOwnerOrSprite) {
    AnimationComponent unattached(kFrames, 0.25f, true);
    EXPECT_NO_THROW(unattached.update(0.5f));

    Entity entity(1);
    auto& animation = entity.addComponent<AnimationComponent>(kFrames, 0.25f, true);
    EXPECT_NO_THROW(animation.update(0.5f));
}

TEST_F(AnimationComponentContractTest, FirstFrameDrawsOnlyItsSourceRectangle) {
    const std::string bmp = CreateTwoFrameBmp();
    ASSERT_FALSE(bmp.empty());
    Renderer renderer(window);
    Entity entity(1);
    auto& transform = entity.addComponent<TransformComponent>();
    transform.width = 16.0f;
    transform.height = 16.0f;
    entity.addComponent<SpriteComponent>(renderer.LoadTexture(bmp.c_str()));
    auto& animation = entity.addComponent<AnimationComponent>(kFrames, 0.25f, true);

    animation.update(0.0f);
    ExpectColor(SampleEntityColor(renderer, entity), 255, 0, 0);
}

TEST_F(AnimationComponentContractTest, AdvancingFrameChangesDisplayedSourceRectangle) {
    const std::string bmp = CreateTwoFrameBmp();
    ASSERT_FALSE(bmp.empty());
    Renderer renderer(window);
    Entity entity(1);
    auto& transform = entity.addComponent<TransformComponent>();
    transform.width = 16.0f;
    transform.height = 16.0f;
    entity.addComponent<SpriteComponent>(renderer.LoadTexture(bmp.c_str()));
    auto& animation = entity.addComponent<AnimationComponent>(kFrames, 0.25f, true);

    animation.update(0.25f);
    ExpectColor(SampleEntityColor(renderer, entity), 0, 255, 0);
}

TEST_F(AnimationComponentContractTest, PausedAnimationDoesNotAdvanceUntilResumed) {
    const std::string bmp = CreateTwoFrameBmp();
    ASSERT_FALSE(bmp.empty());
    Renderer renderer(window);
    Entity entity(1);
    auto& transform = entity.addComponent<TransformComponent>();
    transform.width = 16.0f;
    transform.height = 16.0f;
    entity.addComponent<SpriteComponent>(renderer.LoadTexture(bmp.c_str()));
    auto& animation = entity.addComponent<AnimationComponent>(kFrames, 0.25f, true);

    animation.update(0.0f);
    animation.pause();
    animation.update(0.50f);
    ExpectColor(SampleEntityColor(renderer, entity), 255, 0, 0);

    animation.resume();
    animation.update(0.25f);
    ExpectColor(SampleEntityColor(renderer, entity), 0, 255, 0);
}

TEST_F(AnimationComponentContractTest, LoopingAnimationWrapsToFirstFrame) {
    const std::string bmp = CreateTwoFrameBmp();
    ASSERT_FALSE(bmp.empty());
    Renderer renderer(window);
    Entity entity(1);
    auto& transform = entity.addComponent<TransformComponent>();
    transform.width = 16.0f;
    transform.height = 16.0f;
    entity.addComponent<SpriteComponent>(renderer.LoadTexture(bmp.c_str()));
    auto& animation = entity.addComponent<AnimationComponent>(kFrames, 0.25f, true);

    animation.update(0.50f);
    ExpectColor(SampleEntityColor(renderer, entity), 255, 0, 0);
}

TEST_F(AnimationComponentContractTest, NonLoopingAnimationStaysOnLastFrame) {
    const std::string bmp = CreateTwoFrameBmp();
    ASSERT_FALSE(bmp.empty());
    Renderer renderer(window);
    Entity entity(1);
    auto& transform = entity.addComponent<TransformComponent>();
    transform.width = 16.0f;
    transform.height = 16.0f;
    entity.addComponent<SpriteComponent>(renderer.LoadTexture(bmp.c_str()));
    auto& animation = entity.addComponent<AnimationComponent>(kFrames, 0.25f, false);

    animation.update(1.0f);
    ExpectColor(SampleEntityColor(renderer, entity), 0, 255, 0);
    animation.update(1.0f);
    ExpectColor(SampleEntityColor(renderer, entity), 0, 255, 0);
}

TEST_F(AnimationComponentContractTest, RestartResetsDisplayedFrameAfterUpdate) {
    const std::string bmp = CreateTwoFrameBmp();
    ASSERT_FALSE(bmp.empty());
    Renderer renderer(window);
    Entity entity(1);
    auto& transform = entity.addComponent<TransformComponent>();
    transform.width = 16.0f;
    transform.height = 16.0f;
    entity.addComponent<SpriteComponent>(renderer.LoadTexture(bmp.c_str()));
    auto& animation = entity.addComponent<AnimationComponent>(kFrames, 0.25f, true);

    animation.update(0.25f);
    ExpectColor(SampleEntityColor(renderer, entity), 0, 255, 0);

    animation.restart();
    animation.update(0.0f);
    ExpectColor(SampleEntityColor(renderer, entity), 255, 0, 0);
}

} // namespace
