#include "scene_serializer.h"
#include "sprite.h"
#include "transform.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace {
float readNumber(const nlohmann::json& object, const std::string& key,
                 const std::string& context) {
    if (!object.contains(key) || !object.at(key).is_number()) {
        throw std::runtime_error(context + ": '" + key + "' is missing or not a number");
    }
    return object.at(key).get<float>();
}
} // namespace

std::unique_ptr<Scene> SceneSerializer::load(std::filesystem::path filePath,
                                             AssetManager& assetManager) {

    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open scene file: " + filePath.string());
    }

    nlohmann::json data;
    try {
        data = nlohmann::json::parse(file);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(filePath.string() + ": bad JSON syntax: " + e.what());
    }

    if (!data.contains("version") || !data["version"].is_number_integer() ||
        data["version"] != 1) {
        throw std::runtime_error("Unsupported or missing version in: " + filePath.string());
    }

    if (!data.contains("name") || !data["name"].is_string()) {
        throw std::runtime_error("Unsupported or missing scene name in: " + filePath.string());
    }
    std::string sceneName = data["name"].get<std::string>();

    std::unique_ptr<Scene> scene = std::make_unique<Scene>(sceneName);

    if (!data.contains("entities") || !data["entities"].is_array()) {
        throw std::runtime_error("Entities is missing or not an array: " + filePath.string());
    }

    std::size_t entityIndex = 0;
    for (const auto& entityData : data["entities"]) {
        std::string where = filePath.string() + ": entity " + std::to_string(entityIndex);

        Entity& entity = scene->createEntity();

        // Transform
        if (!entityData.contains("transform") || !entityData.at("transform").is_object()) {
            throw std::runtime_error(where + ": 'transform' is missing or not an object");
        }
        const nlohmann::json& transformData = entityData.at("transform");
        std::string context = where + " transform";

        Transform& transform = entity.addComponent<Transform>();
        transform.x = readNumber(transformData, "x", context);
        transform.y = readNumber(transformData, "y", context);
        transform.width = readNumber(transformData, "width", context);
        transform.height = readNumber(transformData, "height", context);

        // Sprite
        if (!entityData.contains("sprite") || !entityData.at("sprite").is_object()) {
            throw std::runtime_error(where + ": 'sprite' is missing or not an object");
        }
        const nlohmann::json& spriteData = entityData.at("sprite");
        if (!spriteData.contains("texture") || !spriteData.at("texture").is_string()) {
            throw std::runtime_error(where + " sprite: 'texture' is missing or not a string");
        }

        std::string texturePath = spriteData.at("texture").get<std::string>();
        textureHandle handle = assetManager.loadTexture(texturePath);
        entity.addComponent<Sprite>(handle);

        entityIndex++;
    }

    return scene;
}