// Tests for the editor's scene file workflow (P4-124) and Sprite texture edits:
// path validation, Open / Save / Save As, and the unsaved-changes prompt
// (Save, Discard, Cancel, and asking for a path for an untitled scene).
#include "editor_actions.h"
#include "editor_state.h"
#include "scene_path.h"
#include "event_dispatcher.h"
#include "scene.h"
#include "scene_manager.h"
#include "scene_serializer.h"
#include "asset_manager.h"
#include "renderer.h"
#include "transform_component.h"
#include "sprite_component.h"
#include "animation_component.h"

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <system_error>

namespace fs = std::filesystem;

namespace {

std::string SceneJson(const std::string& name, int entityCount) {
    std::string entities;
    for (int i = 0; i < entityCount; ++i) {
        if (i > 0) entities += ",";
        entities += R"({"transform":{"x":1,"y":2,"width":3,"height":4},"sprite":{"texture":"red.bmp"}})";
    }
    return R"({"version":1,"name":")" + name + R"(","entities":[)" + entities + "]}";
}

class SceneFileActionsTest : public ::testing::Test {
  protected:
    SDL_Window* window = nullptr;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<AssetManager> assets;
    fs::path root; // asset root, holds textures and scenes/
    EventDispatcher dispatcher;
    std::unique_ptr<SceneManager> manager;
    SceneSerializer serializer;
    EditorState state;

    void SetUp() override {
        ASSERT_TRUE(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE));
        ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
        window = SDL_CreateWindow("Scene file tests", 64, 64, SDL_WINDOW_HIDDEN);
        ASSERT_NE(window, nullptr) << SDL_GetError();
        renderer = std::make_unique<Renderer>(window);

        std::random_device random;
        for (int attempt = 0; attempt < 100 && root.empty(); ++attempt) {
            auto candidate = fs::temp_directory_path() / ("phase4-scene-files-" + std::to_string(random()));
            if (fs::create_directory(candidate)) root = candidate;
        }
        ASSERT_FALSE(root.empty());
        fs::create_directory(root / "scenes");

        WriteBmp("red.bmp", 4, 4);
        WriteBmp("big.bmp", 64, 64);
        WriteScene("start.json", SceneJson("Start", 1));
        WriteScene("other.json", SceneJson("Other", 2));

        assets = std::make_unique<AssetManager>(*renderer, root);
        manager = std::make_unique<SceneManager>(dispatcher);

        // Begin like the editor does: the start scene is open and clean.
        ASSERT_TRUE(openScene(*manager, serializer, *assets, state, root, "scenes/start.json").success);
    }

    void TearDown() override {
        manager.reset();
        assets.reset();
        renderer.reset();
        if (window != nullptr) SDL_DestroyWindow(window);
        SDL_Quit();
        SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        std::error_code error;
        if (!root.empty()) fs::remove_all(root, error);
    }

    void WriteBmp(const std::string& name, int width, int height) {
        SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
        ASSERT_NE(surface, nullptr) << SDL_GetError();
        ASSERT_TRUE(SDL_SaveBMP(surface, (root / name).string().c_str())) << SDL_GetError();
        SDL_DestroySurface(surface);
    }

    void WriteScene(const std::string& name, const std::string& json) {
        std::ofstream file(root / "scenes" / name, std::ios::binary);
        file << json;
    }

    std::string ActiveName() { return manager->getActiveScene().getSceneName(); }
    std::size_t ActiveCount() { return manager->getActiveScene().getEntities().size(); }

    // Simulates an edit in the editor: one more entity and a dirty scene.
    void MakeEdit() { createEntityAction(manager->getActiveScene(), state); }
};

} // namespace

// ---------------------------------------------------------------- paths

TEST(ScenePathTest, AcceptsJsonFilesInsideScenes) {
    EXPECT_EQ(normalizeScenePath("scenes/level.json").generic_string(), "scenes/level.json");
    EXPECT_EQ(normalizeScenePath("scenes\\level.json").generic_string(), "scenes/level.json");
    EXPECT_EQ(normalizeScenePath("scenes/./sub/level.json").generic_string(), "scenes/sub/level.json");
}

TEST(ScenePathTest, RejectsPathsOutsideScenesOrNotJson) {
    EXPECT_THROW(normalizeScenePath(""), std::invalid_argument);
    EXPECT_THROW(normalizeScenePath("/scenes/level.json"), std::invalid_argument);
    EXPECT_THROW(normalizeScenePath("C:/scenes/level.json"), std::invalid_argument);
    EXPECT_THROW(normalizeScenePath("scenes/../level.json"), std::invalid_argument);
    EXPECT_THROW(normalizeScenePath("levels/level.json"), std::invalid_argument);
    EXPECT_THROW(normalizeScenePath("scenes/level.txt"), std::invalid_argument);
    EXPECT_THROW(normalizeScenePath("scenes"), std::invalid_argument);
}

TEST_F(SceneFileActionsTest, ResolvedPathStaysInsideScenesFolder) {
    const fs::path resolved = resolveScenePath(root, "scenes/start.json");
    EXPECT_EQ(resolved, fs::canonical(root / "scenes" / "start.json"));
}

TEST_F(SceneFileActionsTest, SymlinkEscapingScenesFolderIsRejected) {
    std::ofstream(root / "outside.json") << SceneJson("Outside", 1);
    std::error_code error;
    fs::create_symlink(root / "outside.json", root / "scenes" / "link.json", error);
    if (error) GTEST_SKIP() << "Symlinks not available: " << error.message();

    EXPECT_THROW(resolveScenePath(root, "scenes/link.json"), std::invalid_argument);
    EXPECT_FALSE(openScene(*manager, serializer, *assets, state, root, "scenes/link.json").success);
    EXPECT_EQ(ActiveName(), "Start");
}

// ---------------------------------------------------------------- open

TEST_F(SceneFileActionsTest, OpenReplacesSceneAndResetsEditorState) {
    state.select(1);
    const auto generation = state.getSceneGeneration();

    auto result = openScene(*manager, serializer, *assets, state, root, "scenes/other.json");

    ASSERT_TRUE(result.success) << result.message;
    EXPECT_EQ(ActiveName(), "Other");
    EXPECT_EQ(ActiveCount(), 2u);
    EXPECT_EQ(state.getCurrentScenePath()->generic_string(), "scenes/other.json");
    EXPECT_FALSE(state.isDirty());
    EXPECT_FALSE(state.getSelectedId().has_value());
    EXPECT_EQ(state.getSceneGeneration(), generation + 1);
}

TEST_F(SceneFileActionsTest, FailedOpenKeepsCurrentSceneAndState) {
    WriteScene("broken.json", "{ not json");
    MakeEdit();
    const auto count = ActiveCount();
    const auto generation = state.getSceneGeneration();

    for (const char* path : {"scenes/broken.json", "scenes/missing.json", "../start.json"}) {
        auto result = openScene(*manager, serializer, *assets, state, root, path);
        EXPECT_FALSE(result.success) << path;
        EXPECT_FALSE(result.message.empty()) << path;
    }

    EXPECT_EQ(ActiveName(), "Start");
    EXPECT_EQ(ActiveCount(), count);
    EXPECT_TRUE(state.isDirty());
    EXPECT_EQ(state.getCurrentScenePath()->generic_string(), "scenes/start.json");
    EXPECT_EQ(state.getSceneGeneration(), generation);
}

TEST_F(SceneFileActionsTest, FileActionsAreRefusedWhileRunning) {
    state.resume();
    EXPECT_FALSE(openScene(*manager, serializer, *assets, state, root, "scenes/other.json").success);
    EXPECT_FALSE(saveScene(*manager, serializer, *assets, state, root).success);
    EXPECT_FALSE(saveSceneAs(*manager, serializer, *assets, state, root, "scenes/copy.json").success);
    EXPECT_EQ(requestOpen(*manager, serializer, *assets, state, root, "scenes/other.json").status,
              SceneRequestStatus::Rejected);
    EXPECT_EQ(ActiveName(), "Start");
    EXPECT_FALSE(fs::exists(root / "scenes" / "copy.json"));
}

// ---------------------------------------------------------------- save

TEST_F(SceneFileActionsTest, SaveAsWritesFileAndBecomesCurrentPath) {
    MakeEdit();
    auto result = saveSceneAs(*manager, serializer, *assets, state, root, "scenes/copy.json");

    ASSERT_TRUE(result.success) << result.message;
    EXPECT_TRUE(fs::exists(root / "scenes" / "copy.json"));
    EXPECT_EQ(state.getCurrentScenePath()->generic_string(), "scenes/copy.json");
    EXPECT_FALSE(state.isDirty());
}

TEST_F(SceneFileActionsTest, SaveAsToInvalidPathChangesNothing) {
    MakeEdit();
    EXPECT_FALSE(saveSceneAs(*manager, serializer, *assets, state, root, "../escape.json").success);
    EXPECT_FALSE(saveSceneAs(*manager, serializer, *assets, state, root, "scenes/copy.txt").success);
    EXPECT_FALSE(fs::exists(root / "escape.json"));
    EXPECT_TRUE(state.isDirty());
    EXPECT_EQ(state.getCurrentScenePath()->generic_string(), "scenes/start.json");
}

TEST_F(SceneFileActionsTest, SaveWithoutPathAsksForLocation) {
    state.clearCurrentScenePath();
    MakeEdit();
    auto result = saveScene(*manager, serializer, *assets, state, root);
    EXPECT_FALSE(result.success);
    EXPECT_TRUE(state.isDirty());
}

TEST_F(SceneFileActionsTest, SaveThenOpenRoundTripsEdits) {
    MakeEdit();
    auto& created = *manager->getActiveScene().getEntities().back();
    ASSERT_EQ(renameEntityAction(created, "Player", state), EditResult::Applied);

    ASSERT_TRUE(saveScene(*manager, serializer, *assets, state, root).success);
    ASSERT_TRUE(openScene(*manager, serializer, *assets, state, root, "scenes/other.json").success);
    ASSERT_TRUE(openScene(*manager, serializer, *assets, state, root, "scenes/start.json").success);

    ASSERT_EQ(ActiveCount(), 2u);
    EXPECT_EQ(manager->getActiveScene().getEntities().back()->getName(), "Player");
}

// ---------------------------------------------------------------- unsaved changes

TEST_F(SceneFileActionsTest, OpenOnCleanSceneHappensRightAway) {
    auto result = requestOpen(*manager, serializer, *assets, state, root, "scenes/other.json");
    EXPECT_EQ(result.status, SceneRequestStatus::Opened);
    EXPECT_EQ(ActiveName(), "Other");
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::None);
}

TEST_F(SceneFileActionsTest, OpenOnDirtySceneAsksFirst) {
    MakeEdit();
    auto result = requestOpen(*manager, serializer, *assets, state, root, "scenes/other.json");

    EXPECT_EQ(result.status, SceneRequestStatus::AwaitingConfirmation);
    EXPECT_EQ(ActiveName(), "Start");
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::Open);
    EXPECT_EQ(state.getPendingOpenPath()->generic_string(), "scenes/other.json");

    // A second request while one is waiting is refused.
    EXPECT_EQ(requestOpen(*manager, serializer, *assets, state, root, "scenes/start.json").status,
              SceneRequestStatus::Rejected);
    EXPECT_EQ(state.getPendingOpenPath()->generic_string(), "scenes/other.json");
}

TEST_F(SceneFileActionsTest, InvalidOpenRequestDoesNotLeaveAPendingAction) {
    MakeEdit();
    auto result = requestOpen(*manager, serializer, *assets, state, root, "../start.json");
    EXPECT_EQ(result.status, SceneRequestStatus::Rejected);
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::None);
}

TEST_F(SceneFileActionsTest, CancelKeepsTheEditedScene) {
    MakeEdit();
    const auto count = ActiveCount();
    requestOpen(*manager, serializer, *assets, state, root, "scenes/other.json");

    auto result = resolveUnsavedChanges(*manager, serializer, *assets, state, root, UnsavedChangesChoice::Cancel);

    EXPECT_EQ(result.status, SceneRequestStatus::Cancelled);
    EXPECT_EQ(ActiveName(), "Start");
    EXPECT_EQ(ActiveCount(), count);
    EXPECT_TRUE(state.isDirty());
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::None);
}

TEST_F(SceneFileActionsTest, DiscardOpensWithoutSaving) {
    MakeEdit();
    requestOpen(*manager, serializer, *assets, state, root, "scenes/other.json");

    auto result = resolveUnsavedChanges(*manager, serializer, *assets, state, root, UnsavedChangesChoice::Discard);

    EXPECT_EQ(result.status, SceneRequestStatus::Opened);
    EXPECT_EQ(ActiveName(), "Other");
    EXPECT_FALSE(state.isDirty());

    // The edit was thrown away: start.json still has one entity.
    ASSERT_TRUE(openScene(*manager, serializer, *assets, state, root, "scenes/start.json").success);
    EXPECT_EQ(ActiveCount(), 1u);
}

TEST_F(SceneFileActionsTest, SaveChoiceSavesThenOpens) {
    MakeEdit();
    requestOpen(*manager, serializer, *assets, state, root, "scenes/other.json");

    auto result = resolveUnsavedChanges(*manager, serializer, *assets, state, root, UnsavedChangesChoice::Save);

    EXPECT_EQ(result.status, SceneRequestStatus::Opened);
    EXPECT_EQ(ActiveName(), "Other");

    // The edit was written to start.json before switching.
    ASSERT_TRUE(openScene(*manager, serializer, *assets, state, root, "scenes/start.json").success);
    EXPECT_EQ(ActiveCount(), 2u);
}

TEST_F(SceneFileActionsTest, ResolveWithNothingPendingIsRejected) {
    EXPECT_EQ(resolveUnsavedChanges(*manager, serializer, *assets, state, root, UnsavedChangesChoice::Save).status,
              SceneRequestStatus::Rejected);
    EXPECT_EQ(provideSavePath(*manager, serializer, *assets, state, root, "scenes/x.json").status,
              SceneRequestStatus::Rejected);
}

// ---------------------------------------------------------------- close

TEST_F(SceneFileActionsTest, CloseOnCleanSceneIsReadyRightAway) {
    EXPECT_EQ(requestClose(state).status, SceneRequestStatus::ReadyToClose);
}

TEST_F(SceneFileActionsTest, CloseOnDirtySceneAsksAndPauses) {
    MakeEdit();
    state.resume();

    auto result = requestClose(state);

    EXPECT_EQ(result.status, SceneRequestStatus::AwaitingConfirmation);
    EXPECT_TRUE(state.getPause());
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::Close);

    EXPECT_EQ(resolveUnsavedChanges(*manager, serializer, *assets, state, root, UnsavedChangesChoice::Discard).status,
              SceneRequestStatus::ReadyToClose);
}

// ---------------------------------------------------------------- untitled scene

TEST_F(SceneFileActionsTest, UntitledSceneAsksForPathThenSavesAndCloses) {
    state.clearCurrentScenePath();
    MakeEdit();
    requestClose(state);

    auto ask = resolveUnsavedChanges(*manager, serializer, *assets, state, root, UnsavedChangesChoice::Save);
    EXPECT_EQ(ask.status, SceneRequestStatus::AwaitingSavePath);
    EXPECT_TRUE(state.isAwaitingSavePath());

    auto done = provideSavePath(*manager, serializer, *assets, state, root, "scenes/untitled.json");
    EXPECT_EQ(done.status, SceneRequestStatus::ReadyToClose);
    EXPECT_TRUE(fs::exists(root / "scenes" / "untitled.json"));
    EXPECT_FALSE(state.isDirty());
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::None);
}

TEST_F(SceneFileActionsTest, UntitledSaveCannotOverwriteTheSceneBeingOpened) {
    state.clearCurrentScenePath();
    MakeEdit();
    requestOpen(*manager, serializer, *assets, state, root, "scenes/other.json");
    resolveUnsavedChanges(*manager, serializer, *assets, state, root, UnsavedChangesChoice::Save);

    auto result = provideSavePath(*manager, serializer, *assets, state, root, "scenes/other.json");

    EXPECT_EQ(result.status, SceneRequestStatus::Rejected);
    EXPECT_EQ(ActiveName(), "Start");
    EXPECT_TRUE(state.isDirty());
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::None);

    // other.json still has its original two entities.
    ASSERT_TRUE(openScene(*manager, serializer, *assets, state, root, "scenes/other.json").success);
    EXPECT_EQ(ActiveCount(), 2u);
}

TEST_F(SceneFileActionsTest, InvalidSavePathCancelsThePendingAction) {
    state.clearCurrentScenePath();
    MakeEdit();
    requestClose(state);
    resolveUnsavedChanges(*manager, serializer, *assets, state, root, UnsavedChangesChoice::Save);

    auto result = provideSavePath(*manager, serializer, *assets, state, root, "../escape.json");

    EXPECT_EQ(result.status, SceneRequestStatus::Rejected);
    EXPECT_FALSE(fs::exists(root / "escape.json"));
    EXPECT_TRUE(state.isDirty());
    EXPECT_EQ(state.getPendingAction(), PendingSceneAction::None);
}

// ---------------------------------------------------------------- sprite texture edits

TEST_F(SceneFileActionsTest, SpriteEditWithoutSpriteIsRejected) {
    Entity entity(99);
    EXPECT_EQ(applySpriteEdit(entity, *assets, "big.bmp"), EditResult::Rejected);
}

TEST_F(SceneFileActionsTest, SpriteEditAppliesNewTextureOnce) {
    Entity entity(99);
    auto& sprite = entity.addComponent<SpriteComponent>(assets->loadTexture("red.bmp"));

    EXPECT_EQ(applySpriteEdit(entity, *assets, "red.bmp"), EditResult::NoChange);
    EXPECT_EQ(applySpriteEdit(entity, *assets, "big.bmp"), EditResult::Applied);
    EXPECT_EQ(sprite.getTexture(), assets->loadTexture("big.bmp"));
    EXPECT_EQ(applySpriteEdit(entity, *assets, "big.bmp"), EditResult::NoChange);
}

TEST_F(SceneFileActionsTest, SpriteEditWithBadPathKeepsOldTexture) {
    Entity entity(99);
    const auto red = assets->loadTexture("red.bmp");
    auto& sprite = entity.addComponent<SpriteComponent>(red);

    EXPECT_EQ(applySpriteEdit(entity, *assets, "missing.bmp"), EditResult::Rejected);
    EXPECT_EQ(applySpriteEdit(entity, *assets, "../red.bmp"), EditResult::Rejected);
    EXPECT_EQ(applySpriteEdit(entity, *assets, ""), EditResult::Rejected);
    EXPECT_EQ(sprite.getTexture(), red);
}

TEST_F(SceneFileActionsTest, SpriteEditRejectsTextureTooSmallForAnimationFrames) {
    Entity entity(99);
    const auto big = assets->loadTexture("big.bmp");
    auto& sprite = entity.addComponent<SpriteComponent>(big);
    entity.addComponent<AnimationComponent>(std::vector<Bounds>{{0, 0, 32, 32}, {32, 0, 32, 32}}, 0.1f, true);

    // 4x4 texture cannot hold 32x32 frames.
    EXPECT_EQ(applySpriteEdit(entity, *assets, "red.bmp"), EditResult::Rejected);
    EXPECT_EQ(sprite.getTexture(), big);
}
