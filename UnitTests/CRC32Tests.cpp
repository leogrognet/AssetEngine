#include <gtest/gtest.h>
#include <array>
#include <string_view>
#include "CRC32.h"

TEST(CRC32Test, StandardIEEE8023Vector)
{
    const std::string_view sample = "123456789";
    EXPECT_EQ(CRC32::Compute(std::as_bytes(std::span(sample))), 0xCBF43926u);
}

TEST(CRC32Test, EmptyBufferAndSingleBitSensitivity)
{
    std::span<const std::byte> empty;
    EXPECT_EQ(CRC32::Compute(empty), 0x00000000u);

    std::array<std::byte, 4> data{ std::byte{0x10}, std::byte{0x20}, std::byte{0x30}, std::byte{0x40} };
    const uint32_t originalCrc = CRC32::Compute(data);

    data[2] ^= std::byte{ 0x01 };
    EXPECT_NE(CRC32::Compute(data), originalCrc);
}