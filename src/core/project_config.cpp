#include "project_config.h"
#include "scene_path.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>
#include <stdexcept>


using json = nlohmann::json;

ProjectConfig loadProjectConfig(std::filesystem::path assetRoot) {
    std::filesystem::path configPath = assetRoot / "project.json";

    try {
        std::ifstream readConfigFile(configPath.generic_string());

        if (!readConfigFile.is_open()){
            throw std::runtime_error("Opening project configuration file was unsuccessful : " + configPath.string());
        }

        json jsonParse = json::parse(readConfigFile);

        if (!jsonParse.is_object()) {
            throw std::runtime_error("Invalid project configuration : " + configPath.string());
        }

        if (!jsonParse.contains("version") || !jsonParse["version"].is_number_integer() || jsonParse["version"] != 1) {
            throw std::runtime_error("Unsupported or missing version in: " + configPath.string());
        }

        if (!jsonParse.contains("startupScene") || !jsonParse["startupScene"].is_string() || jsonParse["startupScene"] == ""){
            throw std::runtime_error("Unsupported or missing startupScene in: " + configPath.string());
        }

        std::filesystem::path normalizedPath = normalizeScenePath(jsonParse["startupScene"].get<std::string>());

        std::filesystem::path resolvedPath = resolveScenePath(assetRoot, normalizedPath.generic_string());

        if (!std::filesystem::is_regular_file(resolvedPath)) {
            throw std::runtime_error("Startup scene file does not exist or is a directory : " + resolvedPath.string());
        }

        return {jsonParse["version"].get<int>(), normalizedPath};


    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("Error while loading or parsing JSON : ") + e.what() + "\nConfig file path : " + configPath.string());
    }
}