#include "renderer.h"
#include "logger.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <cmath>

Renderer::Renderer(SDL_Window* window) {
    renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        throw std::runtime_error(std::string("Renderer error: ") + std::string(SDL_GetError()));
    }
    logMessage(logger_level::Info, "Renderer is initialized");
    SDL_SetRenderVSync(renderer, 1);
    textures.push_back(nullptr); // initialize the first texture to nullptr
}

Renderer::~Renderer() {

    for (int i = textures.size() - 1; i > 0; i--) {
        if (textures[i] != nullptr) {

            SDL_DestroyTexture(textures[i]);
            textures[i] = nullptr;
        }
    }

    SDL_DestroyRenderer(renderer);
}

void Renderer::Present() {
    SDL_RenderPresent(renderer);
}

void Renderer::ClearScreen(const Color& color) {
    SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, color.alpha);
    SDL_RenderClear(renderer);
}

void Renderer::DrawFilledRect(const Color& color, const Bounds& bounds) {
    // Pick the colour to draw with
    SDL_SetRenderDrawColor(renderer, color.red, color.green, color.blue, color.alpha);

    // Convert our Bounds into SDL's rectangle type
    SDL_FRect rect;
    rect.x = bounds.x;
    rect.y = bounds.y;
    rect.w = bounds.w;
    rect.h = bounds.h;

    // Draw it (SDL wants the rectangle's address)
    SDL_RenderFillRect(renderer, &rect);
}

textureHandle Renderer::LoadTexture(const char* filePath) {

    // Load the BMP image into an SDL_Surface
    SDL_Surface* surface = SDL_LoadBMP(filePath);

    if (surface == nullptr) {
        throw std::runtime_error(std::string("Failed to load texture:") +
                                 std::string(SDL_GetError()) + std::string(" File: ") +
                                 std::string(filePath));
    }

    // Create a texture from the surface
    SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, surface);

    // free the surface
    SDL_DestroySurface(surface);

    // Check if the texture was created successfully
    if (newTexture == nullptr) {
        throw std::runtime_error(std::string("Failed to create texture:") +
                                 std::string(SDL_GetError()) + std::string(" File: ") +
                                 std::string(filePath));
    }
    textures.push_back(newTexture);
    return static_cast<textureHandle>(textures.size() - 1);
}


void Renderer::DrawTexture(textureHandle handle, const Bounds& bounds,
                           std::optional<Bounds> src) {
    // Check that the texture handle is valid.
    if (handle == 0 || handle >= textures.size()) {
        if (!hasWarned) {
            logMessage(logger_level::Warning, "Invalid texture handle");
            hasWarned = true;
        }
        return;
    }

    SDL_Texture* texture = textures[handle];
    if (texture == nullptr) {
        return;
    }

    // Destination: where the texture will appear on screen.
    SDL_FRect destRect{
        bounds.x, bounds.y, bounds.w, bounds.h
    };

    // No source rectangle means draw the entire texture.
    if (!src.has_value()) {
        SDL_RenderTexture(renderer, texture, nullptr, &destRect);
        return;
    }

    // Get the original texture dimensions.
    float textureWidth = 0.0f;
    float textureHeight = 0.0f;

    if (!SDL_GetTextureSize(texture, &textureWidth, &textureHeight)) {
        logMessage(logger_level::Warning,
                   "Could not get texture dimensions");
        return;
    }

    // Check that the selected frame is valid and inside the texture.
    if (!std::isfinite(src->x) || !std::isfinite(src->y) ||
        !std::isfinite(src->w) || !std::isfinite(src->h) ||
        src->x < 0 || src->y < 0 ||
        src->w <= 0 || src->h <= 0 ||
        src->x + src->w > textureWidth ||
        src->y + src->h > textureHeight) {

        logMessage(logger_level::Warning,
                   "Invalid texture source rectangle");
        return;
    }

    // Source: which part of the texture should be drawn.
    SDL_FRect srcRect{
        src->x, src->y, src->w, src->h
    };

    SDL_RenderTexture(renderer, texture, &srcRect, &destRect);
}


std::pair<float,float> Renderer::getTextureDimensions(textureHandle textureHandle){
    if(textureHandle == 0 || textureHandle >= textures.size()){
        throw std::invalid_argument(" Invalid texture");
    }

    SDL_Texture* texture =  textures[textureHandle];

    float w;
    float h;


    bool success = SDL_GetTextureSize(texture, &w, &h);

    if (!success) {
        throw std::runtime_error("Failed to get texture dimensions");
    }

    return {w, h};


}
