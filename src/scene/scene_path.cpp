#include "scene_path.h"
#include <algorithm>


// Normalizes and validates an asset-relative scene path.
std::filesystem::path normalizeScenePath(const std::string& relativePath) {

    // Standardize path separators.
    std::string stringPath = relativePath;
    std::replace(stringPath.begin(), stringPath.end(), '\\', '/');

    // Reject invalid paths.
    if (stringPath.empty()) {
        throw std::invalid_argument("Empty paths are not allowed");
    }

    if (stringPath.find('\0') != std::string::npos) {
        throw std::invalid_argument("Null characters are not allowed in paths");
    }

    if (stringPath.find(':') != std::string::npos) {
        throw std::invalid_argument("Colons are not allowed in paths");
    }

    std::filesystem::path standardizedPath = stringPath;

    if (standardizedPath.is_absolute()) {
        throw std::invalid_argument("Absolute paths are not allowed");
    }

    if (stringPath.compare(0, 2, "//") == 0 || stringPath[0] == '/') {
        throw std::invalid_argument("Absolute or rooted paths are not allowed");
    }

    for (const auto& component : standardizedPath) {
        if (component == "..") {
            throw std::invalid_argument(".. traversals in paths are not allowed");
        }
    }

    std::filesystem::path lexicallyNormalPath = standardizedPath.lexically_normal();

    if (lexicallyNormalPath.empty() || lexicallyNormalPath == ".") {
        throw std::invalid_argument("Empty paths are not allowed");
    }

    auto iterator = lexicallyNormalPath.begin();

    if (*iterator != "scenes") {
        throw std::invalid_argument("Paths must be in the scenes directory");
    }

    if (lexicallyNormalPath.extension() != ".json") {
        throw std::invalid_argument("Only JSON scene files are allowed");
    }

    return lexicallyNormalPath;
}


// Resolves a relative scene path while ensuring it stays in the scenes directory.
std::filesystem::path resolveScenePath(const std::filesystem::path& assetRoot, const std::string& relativePath) {

    std::filesystem::path normalizedScenePath = normalizeScenePath(relativePath);
    std::filesystem::path canonicalRoot = std::filesystem::canonical(assetRoot);
    std::filesystem::path fullPath = canonicalRoot / normalizedScenePath;
    std::filesystem::path resolvedPath = std::filesystem::weakly_canonical(fullPath);
    std::filesystem::path expectedDirectory = canonicalRoot / "scenes";

    auto expectedIterator = expectedDirectory.begin();
    auto resolvedIterator = resolvedPath.begin();

    while (expectedIterator != expectedDirectory.end()) {
        if (resolvedIterator == resolvedPath.end() || *resolvedIterator != *expectedIterator) {
            throw std::invalid_argument("Path escapes the scenes directory");
        }

        expectedIterator++;
        resolvedIterator++;
    }

    if (resolvedIterator == resolvedPath.end()) {
        throw std::invalid_argument("Path must point to a scene file");
    }

    return resolvedPath;
}
