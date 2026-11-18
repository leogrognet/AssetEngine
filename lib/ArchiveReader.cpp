#include "ArchiveReader.h"

#include "ArchiveFormat.h"
#include "BinaryReader.h"
#include "FileIO.h"

bool ArchiveReader::HasAsset(uint32_t id) const
{
    return m_assets.contains(id);
}

std::span<const std::byte> ArchiveReader::GetAssetData(uint32_t id) const
{
    if (!HasAsset(id))
        throw std::invalid_argument("Asset ID not found");

   std::span<const std::byte> allData(m_buffer );

    return allData.subspan(m_assets.at(id).offset, m_assets.at(id).size);
}


void ArchiveReader::Load(const std::string &path)
{
    std::vector<std::byte> buffer = LoadFile(path);
    BinaryReader reader(buffer);

    std::string magic;
    for (size_t i = 0; i < MAGIC.size(); ++i)
    {
        magic += static_cast<char>(reader.ReadInt<uint8_t>());
    }

    if (magic != MAGIC)
        throw std::runtime_error("Invalid archive format: magic string mismatch.");

    uint32_t version;
    version = reader.ReadInt<uint32_t>();

    if (version != VERSION)
        throw std::runtime_error("Invalid archive format: version mismatch.");

    uint32_t AssetCount;
    AssetCount = reader.ReadInt<uint32_t>();

    uint64_t RegistryOffset;
    RegistryOffset = reader.ReadInt<uint64_t>();
    if (!(RegistryOffset >= HEADER_SIZE && RegistryOffset <= buffer.size()))
        throw std::runtime_error("Invalid archive format: registry offset out of bounds.");
    if ((buffer.size() - RegistryOffset) / ENTRY_SIZE < AssetCount)
        throw std::runtime_error("Invalid archive format: not enough data for asset registry.");

    BinaryReader registryReader(std::span<const std::byte>(buffer.data() + RegistryOffset, AssetCount * ENTRY_SIZE));

    std::unordered_map<uint32_t, ArchiveEntry> assets;
    for (uint32_t i = 0; i < AssetCount; ++i)
    {
        ArchiveEntry entry;
        entry.id = registryReader.ReadInt<uint32_t>();
        entry.type = registryReader.ReadInt<uint32_t>();
        entry.offset = registryReader.ReadInt<uint64_t>();
        entry.size = registryReader.ReadInt<uint64_t>();


        if (entry.offset > buffer.size())
            throw std::runtime_error("Invalid archive format: asset offset out of bounds.");

        if (entry.offset < HEADER_SIZE)
            throw std::runtime_error("Invalid archive format: asset offset overlaps with header or registry.");
        if (entry.size > buffer.size() - entry.offset)
            throw std::runtime_error("Invalid archive format: asset data out of bounds.");

        if (assets.contains(entry.id))
            throw std::runtime_error("Invalid archive format: duplicate asset ID found.");

        assets[entry.id] = entry;
    }

    m_assets = std::move(assets);
    m_buffer = std::move(buffer);

}
