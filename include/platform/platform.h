#pragma once

#include "rendertypes.h"
#include "scene.h"
#include <filesystem>

class Renderer;

bool initializePlatform();
void shutdownPlatform();
void renderPlatform(Scene& scene);
textureHandle loadPlatformTexture(const char* filePath);
std::filesystem::path getAssetRootPath();

