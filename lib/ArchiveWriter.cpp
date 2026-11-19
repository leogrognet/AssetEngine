#include "ArchiveWriter.h"

#include <stdexcept>

#include "ArchiveFormat.h"
#include "BinaryWriter.h"
#include "FileIO.h"
#include "CRC32.h"


bool ArchiveWriter::HasAsset(uint32_t id) const
{
    return assets.contains(id);
}

void ArchiveWriter::AddAsset(uint32_t id, uint32_t type, std::vector<std::byte> data)
{
   if (HasAsset(id))
        throw std::invalid_argument("Asset ID already exists");

    assets[id] = {id, type, std::move(data)};
}

void ArchiveWriter::AddAsset(uint32_t id, uint32_t type, std::span<const std::byte> data)
{
    AddAsset(id, type, std::vector<std::byte>(data.begin(), data.end()));
}

void ArchiveWriter::AddAsset(uint32_t id, AssetType type, std::vector<std::byte> data)
{
    AddAsset(id, static_cast<uint32_t>(type), std::move(data));
}

void ArchiveWriter::AddAsset(uint32_t id, AssetType type, std::span<const std::byte> data)
{
    AddAsset(id, static_cast<uint32_t>(type), data);
}

bool ArchiveWriter::RemoveAsset(uint32_t id)
{
    return assets.erase(id) > 0;
}

void ArchiveWriter::ReplaceAsset(uint32_t id, uint32_t type, std::span<const std::byte> data)
{
    if (!HasAsset(id))
        throw std::invalid_argument("Asset ID not found");

    assets[id] = {id, type, std::vector<std::byte>(data.begin(), data.end())};
}

void ArchiveWriter::ReplaceAsset(uint32_t id, AssetType type, std::span<const std::byte> data)
{
    ReplaceAsset(id, static_cast<uint32_t>(type), data);
}


std::vector<std::byte> ArchiveWriter::Build(uint32_t version) const
{
    BinaryWriter writer;

    for (char c : MAGIC)
        writer.WriteInt<uint8_t>(static_cast<uint8_t>(c));

    writer.WriteInt<uint32_t>(version);
    writer.WriteInt<uint32_t>(static_cast<uint32_t>(assets.size()));
    writer.WriteInt<uint64_t>(HEADER_SIZE);

    uint64_t offset = HEADER_SIZE + assets.size() * ENTRY_SIZE;
    for (const auto& [id, asset] : assets)
    {
        writer.WriteInt<uint32_t>(id);
        writer.WriteInt<uint32_t>(asset.type);
        writer.WriteInt<uint64_t>(offset);
        writer.WriteInt<uint64_t>(asset.data.size());
        offset += asset.data.size();
    }

    for (const auto& [id, asset] : assets)
    {
        writer.WriteBytes(asset.data);
    }

    const uint32_t crc = CRC32::Compute(writer.GetBuffer());
    writer.WriteInt<uint32_t>(crc);

    return writer.TakeBuffer();
}

void ArchiveWriter::Save(const std::string &path) const
{
    SaveFile(path, Build());
}
