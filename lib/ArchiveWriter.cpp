#include "ArchiveWriter.h"

#include <stdexcept>

#include "ArchiveFormat.h"
#include "BinaryWriter.h"
#include "FileIO.h"


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

std::vector<std::byte> ArchiveWriter::Build() const
{
    BinaryWriter writer;

    for (char c : MAGIC)
        writer.WriteInt<uint8_t>(static_cast<uint8_t>(c));

    writer.WriteInt<uint32_t>(VERSION);
    writer.WriteInt<uint32_t>(static_cast<uint32_t>(assets.size()));
    writer.WriteInt<uint64_t>(HEADER_SIZE);

    uint32_t offset = HEADER_SIZE + assets.size() * ENTRY_SIZE;
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


    return writer.TakeBuffer();
}

void ArchiveWriter::Save(const std::string &path) const
{
    SaveFile(path, Build());
}
