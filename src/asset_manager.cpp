#include "asset_manager.h"
#include "renderer.h"
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <cctype>

AssetManager::AssetManager(Renderer& renderer, std::filesystem::path assetRoot): renderer(renderer), assetRoot(std::move(assetRoot)) {}

std::filesystem::path AssetManager::normalizePath(std::filesystem::path relativeAssetPath) const{
    // Standardizing separators -> Only using "/" instead of "\"
    std::string stringPath = relativeAssetPath.generic_string();
    std::replace(stringPath.begin(), stringPath.end(), '\\', '/');

    // Reject invalid paths
    if (stringPath.empty()){
        throw std::invalid_argument("Empty paths are not allowed");
    }

    std::filesystem::path standardizedPath = stringPath;
    if (standardizedPath.is_absolute()){
        throw std::invalid_argument("Absolute paths are not allowed");
    }

    if (stringPath.length() >= 2 && std::isalpha(static_cast<unsigned char>(stringPath[0])) && stringPath[1] == ':') {
        throw std::invalid_argument("Paths starting with Windows drive prefixes are not allowed");
    }

    if (stringPath.compare(0, 2, "//") == 0 || stringPath[0] == '/') {
        throw std::invalid_argument("Absolute or rooted paths are not allowed");
    }

    for (const auto& component : standardizedPath) {
        if (component == ".."){
            throw std::invalid_argument(".. traversals in paths are not allowed");
        }
    }

    std::filesystem::path lexicallyNormalPath = standardizedPath.lexically_normal();

    if (lexicallyNormalPath.empty() || lexicallyNormalPath == "."){
        throw std::invalid_argument("Empty paths are not allowed");
    }

    return lexicallyNormalPath;
}

textureHandle AssetManager::loadTexture(std::filesystem::path relativeAssetPath) {
    std::filesystem::path normalizedPath = this->normalizePath(relativeAssetPath);

    std::string cacheKey = normalizedPath.generic_string();

    auto cacheHit = pathToHandle.find(cacheKey);

    // If texture already exists in pathToHandle, return the handle
    if (cacheHit != pathToHandle.end()){
        return cacheHit->second;
    } 

    // Otherwise gets the full path, add the handle and normalized paths to both maps, and return the handle
    std::filesystem::path fullPath = assetRoot / normalizedPath;
    textureHandle handle = renderer.LoadTexture(fullPath.string().c_str());


    bool insertedSuccessfully = pathToHandle.emplace(cacheKey, handle).second;
    if (!insertedSuccessfully) {
            throw std::logic_error("Adding the new entry in pathToHandle was unsuccesful");
        }
    try {
        if (!handleToPath.emplace(handle, cacheKey).second) {
            throw std::logic_error("Adding the new entry in handleToPath was unsuccesful");
        }
    } catch (...) {
        pathToHandle.erase(cacheKey);
        throw;
    }


    return handle;
}

std::string AssetManager::getPath(textureHandle handle) const {
    auto cacheHit = handleToPath.find(handle);
    if (cacheHit == handleToPath.end()){
        throw std::out_of_range("No texture path associated with the passed handle");
    }
    return cacheHit->second;
}

std::pair<int, int> AssetManager::getOriginalDimensions(textureHandle handle) const {
    auto cacheHit = handleToPath.find(handle);
    if (cacheHit == handleToPath.end()){
        throw std::out_of_range("No texture path associated with the passed handle");
    }
    // Call Renderer function to get dimensions
    // Return said dimensions as a pair of integers

    std::pair<int, int> dimensions = renderer.getTextureDimensions(handle);
    
    return dimensions;

}