#pragma once

#include <filesystem>

struct ProjectConfig {
    int version;
    std::filesystem::path startupScene;
};

ProjectConfig loadProjectConfig(std::filesystem::path assetRoot);