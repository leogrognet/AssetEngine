#pragma once
#include <vector>
#include <cstdint>
#include <map>
#include <span>

struct Asset
{
    uint32_t id;
    uint32_t type;
    std::vector<std::byte> data;
};


class ArchiveWriter
{
public:
    bool HasAsset(uint32_t id) const;



    void AddAsset(uint32_t id, uint32_t type, std::vector<std::byte> data);
    void AddAsset(uint32_t id, uint32_t type, std::span<const std::byte> data);

    bool RemoveAsset(uint32_t id);

    void ReplaceAsset(uint32_t id, uint32_t type, std::span<const std::byte> data);

    [[nodiscard]] std::vector<std::byte> Build() const;

private:
    std::map<uint32_t, Asset> assets;
};
