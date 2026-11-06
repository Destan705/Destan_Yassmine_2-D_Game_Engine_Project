#include "platform.h"
#include "logger.h"
#include "render_api.h"
#include "renderer.h"
#include <SDL3/SDL.h>
#include "asset_manager.h"
#include <iostream>
#include <memory>
#include <stdexcept>

SDL_Window* window = nullptr;
std::unique_ptr<Renderer> renderer = nullptr;
std::unique_ptr<AssetManager> assetManager = nullptr;
textureHandle currentTexture = 0;
//decides whether the game is built with the editor or without
#ifdef ENGINE_EDITOR
#include "editor.h"
std::unique_ptr<Editor> editor = nullptr;
#endif


bool initializePlatform() {
    // Initialize SDL Subsystems
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        logMessage(logger_level::Error,
                   "SDL could not initialize! SDL_Error: " + std::string(SDL_GetError()));
        SDL_Quit();
        return false;
    }
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        logMessage(logger_level::Warning,
                   "SDL could not initialize! (continuing without sound) SDL_Error : " + std::string(SDL_GetError()));
    }
    logMessage(logger_level::Info, "SDL3 initialized successfully!");

    // Create a window
    window = SDL_CreateWindow("My SDL3 Window", 800, 600, SDL_WINDOW_OPENGL);

    if (window == nullptr) {
        logMessage(logger_level::Error,
                   "Window could not be created! SDL_Error: " + std::string(SDL_GetError()));
        SDL_Quit();
        return false;
    }
    logMessage(logger_level::Info, "Window created successfully!");

    // Open Renderer

    try {
        renderer = std::make_unique<Renderer>(window);
        const char* basePath = SDL_GetBasePath();

        if (basePath == nullptr) {
            throw std::runtime_error(std::string("Could not locate assets! SDL_Error: ") +
                                     SDL_GetError());
        }
        const std::string texturePath = std::string(basePath) + "assets/test.bmp";

        currentTexture = renderer->LoadTexture(texturePath.c_str());

            
        std::filesystem::path assetRoot = getAssetRootPath();
        assetManager = std::make_unique<AssetManager>(*renderer, assetRoot);
        #ifdef ENGINE_EDITOR

        editor = std::make_unique<Editor>(window,renderer->getSDLRenderer());

        #endif
    } catch (const std::exception& e) {
        logMessage(logger_level::Error, e.what());
        shutdownPlatform();
        return false;
    }


    return true;
}

void beginFrame() {
    if (renderer == nullptr) {
        logMessage(logger_level::Warning, "Renderer is not initialized!");
        return;
    }
    // Clear the screen with a black color
    renderer->ClearScreen(Color{0, 0, 0, 255});
}

void drawFilledRect(Color rectColor, Bounds rectBounds) {
    // Draw a rectangle in the center of the screen
    renderer->DrawFilledRect(rectColor, rectBounds);
}

void drawTexture() {
    renderer->DrawTexture(currentTexture, Bounds{100.0f, 100.0f, 200.0f, 200.0f});
}

void presentFrame() {
    // Present the rendered frame on the screen
    renderer->Present();
}

void shutdownPlatform() {
    
    #ifdef ENGINE_EDITOR
    editor.reset();
    #endif

    //Destroy the asset manager first
    assetManager.reset();

    // Destroy the renderer 
    renderer.reset();

    // Destroy the window
    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    // Quit SDL subsystems
    logMessage(logger_level::Info, "Engine is shutting down");
    SDL_Quit();

}

void renderPlatform(Scene& scene) {

    if (renderer == nullptr) {
        logMessage(logger_level::Warning, "Renderer is not initialized!");
        return;
    }

    renderer->ClearScreen(Color{255, 255, 255, 255});
    scene.render(*renderer);
    renderer->Present();
}

textureHandle loadPlatformTexture(const char* filePath) {
    if (renderer == nullptr) {
        logMessage(logger_level::Warning, "Renderer is not initialized!");
        return 0;
    }

    textureHandle newTexture = renderer->LoadTexture(filePath);
    return newTexture;
}

std::filesystem::path getAssetRootPath() {
    const char* basePath = SDL_GetBasePath();
    if (basePath == nullptr) {
        logMessage(logger_level::Warning, std::string("Unable to fetch base path : ") + SDL_GetError());
        return {};
    }
    return std::filesystem::path(basePath) / "assets";
}

AssetManager& getPlatformAssetManager() {
    if (assetManager == nullptr) {
        throw std::runtime_error("Platform asset manager is not initialized.");
    }
    return *assetManager;
}