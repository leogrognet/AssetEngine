#include "BinaryReader.h"
#include <bit>


bool BinaryReader::ReadBool()
{
    if (pos + sizeof(bool) > m_buffer.size())
        throw std::runtime_error("Not enough data to read a boolean.");

    auto b = ReadInt<uint8_t>();
    if (b > 1) throw std::runtime_error("Invalid boolean value.");
    return b == 1;
}

float BinaryReader::ReadFloat()
{
    if (pos + sizeof(float) > m_buffer.size())
        throw std::runtime_error("Not enough data to read a float.");

    return std::bit_cast<float>(ReadInt<uint32_t>());
}

double BinaryReader::ReadDouble()
{
    if (pos + sizeof(double) > m_buffer.size())
        throw std::runtime_error("Not enough data to read a double.");

    return std::bit_cast<double>(ReadInt<uint64_t>());
}

std::string BinaryReader::ReadString()
{
    auto length = ReadInt<uint32_t>();
    if (pos + length > m_buffer.size())
        throw std::runtime_error("Not enough data to read a string.");

    std::string str(reinterpret_cast<const char*>(m_buffer.data() + pos), length);
    pos += length;
    return str;
}


