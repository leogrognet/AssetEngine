#pragma once
#include <cstdint>
#include <string_view>



inline constexpr std::string_view MAGIC = "ASET";
inline constexpr uint32_t VERSION = 1;

inline constexpr uint64_t HEADER_SIZE = MAGIC.size()/*Size of the magic string*/ + sizeof(uint32_t)/*Version*/ + sizeof(uint32_t)/*Number of assets*/ + sizeof(uint64_t)/*Position of the Asset Registry*/;
static_assert(HEADER_SIZE == 20, "Header size changed: update the format version.");

inline constexpr uint64_t CRC_SIZE = sizeof(uint32_t);


struct ArchiveEntry
{
    uint32_t id;
    uint32_t type;
    uint64_t offset;
    uint64_t size;
};

inline constexpr uint64_t ENTRY_SIZE =
    sizeof(ArchiveEntry::id) + sizeof(ArchiveEntry::type) +
    sizeof(ArchiveEntry::offset) + sizeof(ArchiveEntry::size);

static_assert(ENTRY_SIZE == 24, "Archive entry size changed: update the format version.");
