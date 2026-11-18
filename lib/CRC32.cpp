#include "CRC32.h"
#include <array>

namespace
{
    consteval std::array<uint32_t, 256> GenerateCRC32Table(uint32_t polynomial) noexcept
    {
        std::array<uint32_t, 256> table{};
        for (uint32_t i = 0; i < 256; ++i)
        {
            uint32_t crc = i;
            for (int bit = 0; bit < 8; ++bit)
            {
                crc = (crc & 1u) ? ((crc >> 1) ^ polynomial) : (crc >> 1);
            }
            table[i] = crc;
        }
        return table;
    }

    constexpr std::array<uint32_t, 256> kCrc32Table = GenerateCRC32Table(CRC32::kPolynomial);
}

uint32_t CRC32::Update(uint32_t state, std::span<const std::byte> data) noexcept
{
    for (const std::byte b : data)
    {
        const auto octet = std::to_integer<uint8_t>(b);
        state = kCrc32Table[(state ^ octet) & 0xFFu] ^ (state >> 8);
    }
    return state;
}

uint32_t CRC32::Compute(std::span<const std::byte> data) noexcept
{
    return Finalize(Update(kInitialState, data));
}