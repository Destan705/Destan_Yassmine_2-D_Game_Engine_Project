#pragma once
#include "rendertypes.h"
#include <unordered_map>
#include <filesystem>
#include <string>
#include <utility>

class Renderer;

class AssetManager{


    public:
        explicit AssetManager(Renderer& renderer, std::filesystem::path assetRoot); // Initializes the manager

        textureHandle loadTexture(std::filesystem::path relativeAssetPath); // Returns cached handle or loads the asset

        std::string getPath(textureHandle handle) const; // Finds the original normalized relative path

        std::pair<int, int> getOriginalDimensions(textureHandle handle) const; // Returns original texture dimensions


    private:

        std::unordered_map<std::string, textureHandle> pathToHandle; // Remembers which handle belongs to each normalized path

        std::unordered_map<textureHandle, std::string> handleToPath; // Finds the asset-relative path associated with a handle
        
        Renderer& renderer; // References the renderer that actually owns the textures

        std::filesystem::path assetRoot; // Stores where the assets are located on disk  

        std::filesystem::path normalizePath(std::filesystem::path relativeAssetPath) const; // validates and normalizes a relative asset path so equivalent paths use the same cache key


}; 