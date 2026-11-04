#include "scene_serializer.h"
#include "sprite.h"
#include "transform.h"
#include "animationComponent.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
// Reads one number from a JSON object, or throws a clear error.
float readNumber(const nlohmann::json& object, const std::string& key, const std::string& context) {
    if (!object.contains(key) || !object.at(key).is_number()) {
        throw std::runtime_error(context + ": '" + key + "' is missing or not a number");
    }
    return object.at(key).get<float>();
}

// Typo detector: throws if the object has a key that isn't allowed.
void checkKnownKeys(const nlohmann::json& object, const std::vector<std::string>& allowed,
                    const std::string& context) {
    for (const auto& item : object.items()) {
        if (std::find(allowed.begin(), allowed.end(), item.key()) == allowed.end()) {
            throw std::runtime_error(context + ": unknown field '" + item.key() + "'");
        }
    }
}
} // namespace

std::unique_ptr<Scene> SceneSerializer::load(std::filesystem::path filePath,
                                             AssetManager& assetManager) {

    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open scene file: " + filePath.string());
    }

    // One set of keys per open { ... }, used to spot duplicate keys.
    std::vector<std::set<std::string>> seenKeys;

    // Called by nlohmann for every piece of JSON it reads.
    auto rejectDuplicates = [&](int /*depth*/, nlohmann::json::parse_event_t event,
                                nlohmann::json& parsed) {
        if (event == nlohmann::json::parse_event_t::object_start) {
            seenKeys.push_back({});
        }

        if (event == nlohmann::json::parse_event_t::object_end) {
            seenKeys.pop_back();
        }

        if (event == nlohmann::json::parse_event_t::key) {

            std::string key = parsed.get<std::string>();

            if (!seenKeys.back().insert(key).second) {

                throw std::runtime_error(filePath.string() + ": duplicate key '" + key + "'");
            }
        }

        return true;
    };

    nlohmann::json data;
    try {
        data = nlohmann::json::parse(file, rejectDuplicates);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error(filePath.string() + ": bad JSON syntax: " + e.what());
    }

    if (!data.is_object()) {
        throw std::runtime_error(filePath.string() + ": top level must be an object");
    }
    checkKnownKeys(data, {"version", "name", "entities"}, filePath.string());

    if (!data.contains("version") || !data["version"].is_number_integer() || data["version"] != 1) {
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

        if (!entityData.is_object()) {
            throw std::runtime_error(where + ": must be an object");
        }
        checkKnownKeys(entityData, {"transform", "sprite", "animation"}, where);

        Entity& entity = scene->createEntity();

        // Transform
        if (!entityData.contains("transform") || !entityData.at("transform").is_object()) {
            throw std::runtime_error(where + ": 'transform' is missing or not an object");
        }
        const nlohmann::json& transformData = entityData.at("transform");
        std::string context = where + " transform";
        checkKnownKeys(transformData, {"x", "y", "width", "height"}, context);

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
        checkKnownKeys(spriteData, {"texture"}, where + " sprite");

        if (!spriteData.contains("texture") || !spriteData.at("texture").is_string()) {
            throw std::runtime_error(where + " sprite: 'texture' is missing or not a string");
        }

        std::string texturePath = spriteData.at("texture").get<std::string>();
        textureHandle handle = assetManager.loadTexture(texturePath);
        entity.addComponent<Sprite>(handle);

        if (entityData.contains("animation")) {
               const nlohmann::json& animData = entityData.at("animation");
   checkKnownKeys(animData, {"frames", "frameDuration", "loop"}, where + " animation");
   
         // Animation (optional)
        if (entityData.contains("animation")) {
            std::string animContext = where + " animation";

            if (!entityData.at("animation").is_object()) {
                throw std::runtime_error(animContext + ": must be an object");
            }
            const nlohmann::json& animData = entityData.at("animation");
            checkKnownKeys(animData, {"frames", "frameDuration", "loop"}, animContext);

            // Frames
            if (!animData.contains("frames") || !animData.at("frames").is_array() ||
                animData.at("frames").empty()) {
                throw std::runtime_error(animContext + ": 'frames' must be a non-empty array");
            }

            std::vector<Bounds> frames;
            std::size_t frameIndex = 0;
            for (const auto& frameData : animData.at("frames")) {
                std::string frameContext = animContext + " frame " + std::to_string(frameIndex);
                if (!frameData.is_object()) {
                    throw std::runtime_error(frameContext + ": must be an object");
                }
                checkKnownKeys(frameData, {"x", "y", "w", "h"}, frameContext);

                Bounds frame;
                frame.x = readNumber(frameData, "x", frameContext);
                frame.y = readNumber(frameData, "y", frameContext);
                frame.w = readNumber(frameData, "w", frameContext);
                frame.h = readNumber(frameData, "h", frameContext);
                frames.push_back(frame);
                frameIndex++;
            }

            // Duration
            float duration = readNumber(animData, "frameDuration", animContext);
            if (duration <= 0.0f) {
                throw std::runtime_error(animContext + ": 'frameDuration' must be > 0");
            }

            // Loop
            if (!animData.contains("loop") || !animData.at("loop").is_boolean()) {
                throw std::runtime_error(animContext + ": 'loop' is missing or not true/false");
            }
            bool loop = animData.at("loop").get<bool>();

            entity.addComponent<AnimationComponent>(frames, duration, loop);
        }

        entityIndex++;
    }

    return scene;
}

                                             }

void SceneSerializer::save(const Scene& scene, std::filesystem::path filePath,
                           const AssetManager& assetManager) {
    nlohmann::json data;
    data["version"] = 1;
    data["name"] = scene.getSceneName();

    nlohmann::json entities = nlohmann::json::array();

    for (const auto& entity : scene.getEntities()) {
        nlohmann::json entityData;

        // Transform
        Transform* transform = entity->getComponent<Transform>();
        if (transform == nullptr) {
            throw std::runtime_error("Entity is missing Transform component");
        }
        entityData["transform"]["x"] = transform->x;
        entityData["transform"]["y"] = transform->y;
        entityData["transform"]["width"] = transform->width;
        entityData["transform"]["height"] = transform->height;

        // Sprite
        Sprite* sprite = entity->getComponent<Sprite>();
        if (sprite == nullptr) {
            throw std::runtime_error("Entity is missing Sprite component");
        }
        entityData["sprite"]["texture"] = assetManager.getPath(sprite->getTexture());

        entities.push_back(entityData);
    }

    data["entities"] = entities;

    // Write to a temp file first, so the real file is safe if anything fails.
    std::filesystem::path tempPath = filePath;
    tempPath += ".tmp";

    std::ofstream file(tempPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + tempPath.string());
    }
    file << data.dump(4);
    file.close();

    if (!file) {
        std::filesystem::remove(tempPath);
        throw std::runtime_error("Error writing to file: " + tempPath.string());
    }

    // Swap the finished temp file in place of the real one.
    try {
        std::filesystem::rename(tempPath, filePath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::filesystem::remove(tempPath);
        throw std::runtime_error("Could not replace " + filePath.string() + ": " + e.what());
    }
}