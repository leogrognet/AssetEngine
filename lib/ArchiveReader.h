#pragma once
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "ArchiveFormat.h"

class ArchiveReader
{
public:

    void Load(const std::string& path);

    [[nodiscard]] bool HasAsset(uint32_t id) const;

    [[nodiscard]] std::span<const std::byte> GetAssetData(uint32_t id) const;

    [[nodiscard]] AssetType GetAssetType(uint32_t id) const;

    [[nodiscard]] std::vector<uint32_t> GetAssetsByType(AssetType type) const;
    [[nodiscard]] uint32_t GetVersion() const {return m_version;}

private:
    std::vector<std::byte> m_buffer;
    std::unordered_map<uint32_t, ArchiveEntry> m_assets;
    uint32_t m_version = 0;
};
