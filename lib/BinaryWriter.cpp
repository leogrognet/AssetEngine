#include "../BinaryWriter.h"
#include <bit>
#include <stdexcept>


void BinaryWriter::WriteBool(bool value)
{
    m_buffer.push_back(static_cast<std::byte>(value));
}

void BinaryWriter::WriteFloat(float value)
{
    WriteInt(std::bit_cast<uint32_t>(value));
}
void BinaryWriter::WriteDouble(double value)
{
    WriteInt(std::bit_cast<uint64_t>(value));
}


void BinaryWriter::WriteString(std::string_view str)
{
    if (str.size() > std::numeric_limits<uint32_t>::max())
    {
        throw std::length_error("String is too long to write.");
    }

    WriteInt<uint32_t>(str.size());
    for (char c : str)
    {
        m_buffer.push_back(static_cast<std::byte>(c));
    }
}


