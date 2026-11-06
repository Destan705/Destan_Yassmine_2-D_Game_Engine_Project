
#include "entity.h"
#include "renderer.h"
#include "sprite_component.h"
#include "transform_component.h"

#include <gtest/gtest.h>

#include <SDL3/SDL.h>
#include <cstdint>
#include <stdexcept>

// ============================================================
// Transform tests
// ============================================================

TEST(TransformTest, DefaultsToZero) {
    TransformComponent transform;

    EXPECT_FLOAT_EQ(transform.x, 0.0f);
    EXPECT_FLOAT_EQ(transform.y, 0.0f);
    EXPECT_FLOAT_EQ(transform.width, 0.0f);
    EXPECT_FLOAT_EQ(transform.height, 0.0f);
}

TEST(TransformTest, StoresPositionAndSize) {
    TransformComponent transform;

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
    TransformComponent transform;

    transform.x = 10.0f;
    transform.y = 20.0f;

    transform.x += 5.0f;
    transform.y -= 10.0f;

    EXPECT_FLOAT_EQ(transform.x, 15.0f);
    EXPECT_FLOAT_EQ(transform.y, 10.0f);
}

TEST(TransformTest, CanBeAttachedToEntity) {
    Entity entity(1);

    TransformComponent& transform = entity.addComponent<TransformComponent>();

    EXPECT_EQ(entity.getComponent<TransformComponent>(), &transform);
    EXPECT_EQ(transform.getOwner(), &entity);
}

TEST(TransformTest, DifferentEntitiesHaveIndependentTransforms) {
    Entity first(1);
    Entity second(2);

    first.addComponent<TransformComponent>().x = 25.0f;
    second.addComponent<TransformComponent>().x = 75.0f;

    EXPECT_FLOAT_EQ(first.getComponent<TransformComponent>()->x, 25.0f);
    EXPECT_FLOAT_EQ(second.getComponent<TransformComponent>()->x, 75.0f);
}

// ============================================================
// Sprite tests
// ============================================================

TEST(SpriteTest, CanBeAttachedToEntity) {
    Entity entity(1);

    SpriteComponent& sprite = entity.addComponent<SpriteComponent>(1u);

    EXPECT_EQ(entity.getComponent<SpriteComponent>(), &sprite);
    EXPECT_EQ(sprite.getOwner(), &entity);
}

TEST(SpriteTest, SpriteAndTransformShareSameOwner) {
    Entity entity(1);

    TransformComponent& transform = entity.addComponent<TransformComponent>();
    SpriteComponent& sprite = entity.addComponent<SpriteComponent>(1u);

    EXPECT_EQ(transform.getOwner(), &entity);
    EXPECT_EQ(sprite.getOwner(), &entity);
    EXPECT_EQ(entity.getComponent<TransformComponent>(), &transform);
}

TEST(SpriteTest, CannotAttachDuplicateSprite) {
    Entity entity(1);

    entity.addComponent<SpriteComponent>(1u);

    EXPECT_THROW(entity.addComponent<SpriteComponent>(2u), std::logic_error);
}

TEST(SpriteTest, CannotAttachDuplicateTransform) {
    Entity entity(1);

    entity.addComponent<TransformComponent>();

    EXPECT_THROW(entity.addComponent<TransformComponent>(), std::logic_error);
}

TEST(SpriteTest, MissingTransformCanBeDetected) {
    Entity entity(1);
    entity.addComponent<SpriteComponent>(1u);

    EXPECT_EQ(entity.getComponent<TransformComponent>(), nullptr);
}

TEST(SpriteTest, DifferentEntitiesCanHaveDifferentSprites) {
    Entity first(1);
    Entity second(2);

    SpriteComponent& firstSprite = first.addComponent<SpriteComponent>(1u);
    SpriteComponent& secondSprite = second.addComponent<SpriteComponent>(2u);

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
        entity.addComponent<SpriteComponent>(1u);

        EXPECT_NO_THROW(entity.render(renderer));
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
}
