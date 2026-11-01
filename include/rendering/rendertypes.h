// RenderTypes.hpp
// Small, engine-owned data types used by the rendering interface
// (Color and Bounds). They let callers describe what to draw
// without depending on SDL types; the Renderer converts them to
// SDL types internally.
#pragma once 
#include <cstdint>

using textureHandle = unsigned int;

struct Color {
std::uint8_t red=255;
std::uint8_t green=255;
std::uint8_t blue=255;
std::uint8_t alpha=255;
};

struct Bounds{
    float x=0.0f;
    float y=0.0f;
    float w=0.0f;
    float h=0.0f;
};

