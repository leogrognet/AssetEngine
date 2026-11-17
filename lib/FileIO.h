#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <vector>

[[nodiscard]] std::vector<std::byte> LoadFile(const std::string& path);
void SaveFile(const std::string& path, std::span<const std::byte> data);