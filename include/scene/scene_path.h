#pragma once

#include <filesystem>
#include <string>

std::filesystem::path normalizeScenePath(const std::string& relativePath);

std::filesystem::path resolveScenePath(const std::filesystem::path& assetRoot, const std::string& relativePath);