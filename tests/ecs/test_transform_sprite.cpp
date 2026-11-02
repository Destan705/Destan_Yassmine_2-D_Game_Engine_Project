
#include "entity.h"
#include "renderer.h"
#include "sprite.h"
#include "transform.h"

#include <gtest/gtest.h>

#include <SDL3/SDL.h>
#include <cstdint>
#include <stdexcept>

// ============================================================
// Transform tests
// ============================================================

TEST(TransformTest, DefaultsToZero) {
    Transform transform;

    EXPECT_FLOAT_EQ(transform.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.y, 0.0f);
    EXPECT_FLOAT_EQ(transform.width, 0.0f);
    EXPECT_FLOAT_EQ(transform.height, 0.0f);
}

TEST(TransformTest, StoresPositionAndSize) {
    Transform transform;

    transform.x = 50.0f;
    transform.y = 75.0f;
    transform.width = 100.0f;
    transform.height = 150.0f;

    EXPECT_FLOAT_EQ(transform.x, 50.0f);
    EXPECT_FLOAT_EQ(transform.y, 75.0f);
    EXPECT_FLOAT_EQ(transform.width, 100.0f);
    EXPECT_FLOAT_EQ(transform.height, 150.0f);
}

TEST(TransformTest, ValuesCanBeUpdated) {
    Transform transform;

    transform.x = 10.0f;
    transform.y = 20.0f;

    transform.x += 5.0f;
    transform.y -= 10.0f;

    EXPECT_FLOAT_EQ(transform.x, 15.0f);
    EXPECT_FLOAT_EQ(transform.y, 10.0f);
}

TEST(TransformTest, CanBeAttachedToEntity) {
    Entity entity(1);

    Transform& transform = entity.addComponent<Transform>();

    EXPECT_EQ(entity.getComponent<Transform>(), &transform);
    EXPECT_EQ(transform.getOwner(), &entity);
}

TEST(TransformTest, DifferentEntitiesHaveIndependentTransforms) {
    Entity first(1);
    Entity second(2);

    first.addComponent<Transform>().x = 25.0f;
    second.addComponent<Transform>().x = 75.0f;

    EXPECT_FLOAT_EQ(first.getComponent<Transform>()->x, 25.0f);
    EXPECT_FLOAT_EQ(second.getComponent<Transform>()->x, 75.0f);
}

// ============================================================
// Sprite tests
// ============================================================

TEST(SpriteTest, CanBeAttachedToEntity) {
    Entity entity(1);

    Sprite& sprite = entity.addComponent<Sprite>(1u);

    EXPECT_EQ(entity.getComponent<Sprite>(), &sprite);
    EXPECT_EQ(sprite.getOwner(), &entity);
}

TEST(SpriteTest, SpriteAndTransformShareSameOwner) {
    Entity entity(1);

    Transform& transform = entity.addComponent<Transform>();
    Sprite& sprite = entity.addComponent<Sprite>(1u);

    EXPECT_EQ(transform.getOwner(), &entity);
    EXPECT_EQ(sprite.getOwner(), &entity);
    EXPECT_EQ(entity.getComponent<Transform>(), &transform);
}

TEST(SpriteTest, CannotAttachDuplicateSprite) {
    Entity entity(1);

    entity.addComponent<Sprite>(1u);

    EXPECT_THROW(entity.addComponent<Sprite>(2u), std::logic_error);
}

TEST(SpriteTest, CannotAttachDuplicateTransform) {
    Entity entity(1);

    entity.addComponent<Transform>();

    EXPECT_THROW(entity.addComponent<Transform>(), std::logic_error);
}

TEST(SpriteTest, MissingTransformCanBeDetected) {
    Entity entity(1);
    entity.addComponent<Sprite>(1u);

    EXPECT_EQ(entity.getComponent<Transform>(), nullptr);
}

TEST(SpriteTest, DifferentEntitiesCanHaveDifferentSprites) {
    Entity first(1);
    Entity second(2);

    Sprite& firstSprite = first.addComponent<Sprite>(1u);
    Sprite& secondSprite = second.addComponent<Sprite>(2u);

    EXPECT_NE(&firstSprite, &secondSprite);
    EXPECT_EQ(firstSprite.getOwner(), &first);
    EXPECT_EQ(secondSprite.getOwner(), &second);
}

TEST(SpriteTest, MissingTransformSafelySkipsDrawing) {
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "dummy");
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");

    ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window* window = SDL_CreateWindow("Sprite Test", 320, 240, SDL_WINDOW_HIDDEN);

    ASSERT_NE(window, nullptr);

    {
        Renderer renderer(window);

        Entity entity(1);
        entity.addComponent<Sprite>(1u);

        EXPECT_NO_THROW(entity.render(renderer));
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
}
