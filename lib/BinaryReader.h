#pragma once

#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <fstream>
#include <string>

class BinaryReader
{
public:
    BinaryReader() = default;
    explicit BinaryReader(std::span<const std::byte> data) : m_buffer(data) {}


    template <std::unsigned_integral T>
    T ReadInt()
    {
        if (pos + sizeof(T) > m_buffer.size())
            throw std::runtime_error("Not enough data to read an integer.");

        T value = 0;
        for (size_t i = 0; i < sizeof(T); ++i)
        {
            value |= static_cast<T>(m_buffer[pos + i]) << (8 * i);
        }
        pos += sizeof(T);
        return value;
    }

    template <std::signed_integral T>
    T ReadInt()
    {
        return static_cast<T>(ReadInt<std::make_unsigned_t<T>>());
    }

    bool ReadBool();

    float ReadFloat();
    double ReadDouble();

    std::string ReadString();

    template <typename T>
    std::vector<T> ReadVector()
    {
        auto length = ReadInt<uint32_t>();
        if (length * sizeof(T) > m_buffer.size() - pos)
            throw std::runtime_error("Not enough data to read a vector.");

        std::vector<T> vec(length);
        for (size_t i = 0; i < length; ++i)
        {
            vec[i] = ReadValue<T>();
        }
        return vec;
    }

    template <std::integral T>
    T ReadValue() { return ReadInt<T>(); }

    template <std::same_as<float> T>
    T ReadValue() { return ReadFloat(); }

    template <std::same_as<double> T>
    T ReadValue() { return ReadDouble(); }

    template <std::same_as<std::string> T>
    T ReadValue() { return ReadString(); }

    template <typename T>
        requires requires(BinaryReader& r, T& out) { Deserialize(r, out); }
    T ReadValue()
    {
        T value{};
        Deserialize(*this, value);
        return value;
    }


    [[nodiscard]] std::span<const std::byte> getBuffer() const { return m_buffer; }


private:
    std::span<const std::byte> m_buffer;
    size_t pos = 0;
};


