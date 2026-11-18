#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

class CRC32
{
public:
    static constexpr uint32_t kInitialState = 0xFFFFFFFFu;
    static constexpr uint32_t kPolynomial = 0xEDB88320u;

    static uint32_t Update(uint32_t state, std::span<const std::byte> data) noexcept;
    static constexpr uint32_t Finalize(uint32_t state) noexcept { return ~state; }
    static uint32_t Compute(std::span<const std::byte> data) noexcept;
};