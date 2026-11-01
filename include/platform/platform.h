#pragma once

#include "rendertypes.h"
#include "scene.h"

class Renderer;

bool initializePlatform();
void shutdownPlatform();
void renderPlatform(Scene& scene);
textureHandle loadPlatformTexture(const char* filePath);
