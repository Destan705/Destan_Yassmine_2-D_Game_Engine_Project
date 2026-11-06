#include "scene_serializer.h"
#include "animation_component.h"
#include "collider_component.h"
#include "sprite_component.h"
#include "transform_component.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
// Reads one number from a JSON object, or throws a clear error.

float readNumber(const nlohmann::json& object, const std::string& key,
                 const std::string& context) {
    if (!object.contains(key) || !object.at(key).is_number()) {
        throw std::runtime_error(context + ": '" + key +
                                 "' is missing or not a number");
    }

    float value = object.at(key).get<float>();

    if (!std::isfinite(value)) {
        throw std::runtime_error(context + ": '" + key +
                                 "' must be finite");
    }

    return value;
}


// Colliders require finite values (not NaN or infinity).
float readFiniteNumber(const nlohmann::json& object, const std::string& key,
                       const std::string& context) {
    float value = readNumber(object, key, context);
    if (!std::isfinite(value)) {
        throw std::runtime_error(context + ": '" + key + "' must be finite");
    }
    return value;
}

// Typos and unsupported fields should be reported instead of silently ignored.
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

    // Keep track of object keys while parsing, so duplicate JSON keys are rejected.
    std::vector<std::set<std::string>> seenKeys;
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
    // "collider" is an ENTITY field, not a top-level scene field.
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
        checkKnownKeys(entityData, {"transform", "sprite", "animation", "collider"}, where);

        Entity& entity = scene->createEntity();

        // Transform (required)
        if (!entityData.contains("transform") || !entityData.at("transform").is_object()) {
            throw std::runtime_error(where + ": 'transform' is missing or not an object");
        }
        const nlohmann::json& transformData = entityData.at("transform");
        std::string context = where + " transform";
        checkKnownKeys(transformData, {"x", "y", "width", "height"}, context);

        TransformComponent& transform = entity.addComponent<TransformComponent>();
        transform.x = readNumber(transformData, "x", context);
        transform.y = readNumber(transformData, "y", context);
        transform.width = readNumber(transformData, "width", context);
        transform.height = readNumber(transformData, "height", context);

        // Sprite (required)
        if (!entityData.contains("sprite") || !entityData.at("sprite").is_object()) {
            throw std::runtime_error(where + ": 'sprite' is missing or not an object");
        }
        const nlohmann::json& spriteData = entityData.at("sprite");
        checkKnownKeys(spriteData, {"texture"}, where + " sprite");
        if (!spriteData.contains("texture") || !spriteData.at("texture").is_string()) {
            throw std::runtime_error(where + " sprite: 'texture' is missing or not a string");
        }

                
        std::string texturePath = spriteData.at("texture").get<std::string>();

        textureHandle handle;

        try {
            handle = assetManager.loadTexture(texturePath);
        } catch (const std::exception& e) {
            throw std::runtime_error(
                where + " sprite.texture '" + texturePath +
                "': " + e.what()
            );
        }

        entity.addComponent<SpriteComponent>(handle);


        // Collider (optional). Store local bounds, NOT world-space bounds.
        if (entityData.contains("collider")) {
            const std::string colliderContext = where + " collider";
            if (!entityData.at("collider").is_object()) {
                throw std::runtime_error(colliderContext + ": must be an object");
            }
            const nlohmann::json& colliderData = entityData.at("collider");
            checkKnownKeys(colliderData,
                           {"offsetX", "offsetY", "width", "height", "enabled"},
                           colliderContext);

            float offsetX = readFiniteNumber(colliderData, "offsetX", colliderContext);
            float offsetY = readFiniteNumber(colliderData, "offsetY", colliderContext);
            float width = readFiniteNumber(colliderData, "width", colliderContext);
            float height = readFiniteNumber(colliderData, "height", colliderContext);

            if (width <= 0.0f) {
                throw std::runtime_error(colliderContext + ": 'width' must be greater than zero");
            }
            if (height <= 0.0f) {
                throw std::runtime_error(colliderContext + ": 'height' must be greater than zero");
            }
            if (!colliderData.contains("enabled") || !colliderData.at("enabled").is_boolean()) {
                throw std::runtime_error(colliderContext + ": 'enabled' is missing or not true/false");
            }
            bool enabled = colliderData.at("enabled").get<bool>();

            entity.addComponent<ColliderComponent>(offsetX, offsetY, width, height, enabled);
        }

        // Animation (optional)
        if (entityData.contains("animation")) {
            std::string animContext = where + " animation";
            if (!entityData.at("animation").is_object()) {
                throw std::runtime_error(animContext + ": must be an object");
            }
            const nlohmann::json& animData = entityData.at("animation");
            checkKnownKeys(animData, {"frames", "frameDuration", "loop"}, animContext);

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

                // Animation frames must have positive dimensions.
                if (frame.w <= 0.0f) {
                    throw std::runtime_error(
                        frameContext + ": 'w' must be greater than zero"
                    );
                }

                if (frame.h <= 0.0f) {
                    throw std::runtime_error(
                        frameContext + ": 'h' must be greater than zero"
                    );
                }

                frames.push_back(frame);

                frameIndex++;
            }

            float duration = readNumber(animData, "frameDuration", animContext);
            if (duration <= 0.0f) {
                throw std::runtime_error(animContext + ": 'frameDuration' must be > 0");
            }
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

void SceneSerializer::save(const Scene& scene, std::filesystem::path filePath,
                           const AssetManager& assetManager) {
    nlohmann::json data;
    data["version"] = 1;
    data["name"] = scene.getSceneName();

    nlohmann::json entities = nlohmann::json::array();
    for (const auto& entity : scene.getEntities()) {
        nlohmann::json entityData;

        // Transform (required)
        TransformComponent* transform = entity->getComponent<TransformComponent>();
        if (transform == nullptr) {
            throw std::runtime_error("Entity is missing Transform component");
        }
        entityData["transform"]["x"] = transform->x;
        entityData["transform"]["y"] = transform->y;
        entityData["transform"]["width"] = transform->width;
        entityData["transform"]["height"] = transform->height;

        // Sprite (required)
        SpriteComponent* sprite = entity->getComponent<SpriteComponent>();
        if (sprite == nullptr) {
            throw std::runtime_error("Entity is missing Sprite component");
        }
        entityData["sprite"]["texture"] = assetManager.getPath(sprite->getTexture());

        // Collider (optional). Preserve its local bounds and enabled state.
        ColliderComponent* collider = entity->getComponent<ColliderComponent>();
        if (collider != nullptr) {
            nlohmann::json colliderData;
            colliderData["offsetX"] = collider->getOffsetX();
            colliderData["offsetY"] = collider->getOffsetY();
            colliderData["width"] = collider->getWidth();
            colliderData["height"] = collider->getHeight();
            colliderData["enabled"] = collider->isEnabled();
            entityData["collider"] = colliderData;
        }

        // Animation (optional)
        AnimationComponent* animation = entity->getComponent<AnimationComponent>();
        if (animation != nullptr) {
            nlohmann::json animData;
            animData["frameDuration"] = animation->getDuration();
            animData["loop"] = animation->getRepeat();
            for (const auto& frame : animation->getFrames()) {
                nlohmann::json frameData;
                frameData["x"] = frame.x;
                frameData["y"] = frame.y;
                frameData["w"] = frame.w;
                frameData["h"] = frame.h;
                animData["frames"].push_back(frameData);
            }
            entityData["animation"] = animData;
        }
        entities.push_back(entityData);
    }
    data["entities"] = entities;

    // Write a temporary file before replacing the destination.
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
    try {
        std::filesystem::rename(tempPath, filePath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::filesystem::remove(tempPath);
        throw std::runtime_error("Could not replace " + filePath.string() + ": " + e.what());
    }
}
