#pragma once
#include "asset_manager.h"
#include "scene.h"
#include <filesystem>
#include <memory>
class SceneSerializer {
  public:
    // turns a file in to scene
    std::unique_ptr<Scene> load(std::filesystem::path filePath, AssetManager& assetManager);
    // turns a scene in to a file
    void save(const Scene& scene, const std::filesystem::path filePath,
              const AssetManager& assetManager);
};