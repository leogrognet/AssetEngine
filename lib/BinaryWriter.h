#pragma once

#include <cstdint>
#include <span>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <limits>

class BinaryWriter
{
public:
  BinaryWriter() = default;

  template <std::unsigned_integral T>
  void WriteInt(T value)
  {
    for (size_t i = 0; i < sizeof(T); ++i)
    {
      m_buffer.push_back(static_cast<std::byte>(value & 0xFF));
      value >>= 8;
    }
  }

  template <std::signed_integral T>
  void WriteInt(T value)
  {
    WriteInt(static_cast<std::make_unsigned_t<T>>(value));
  }

  void WriteBool(bool value);

  void WriteFloat(float value);

  void WriteDouble(double value);

  void WriteString(std::string_view str);

  template <typename T>
  void WriteVector(const std::vector<T>& vec)
  {
    if (vec.size() > std::numeric_limits<uint32_t>::max())
      throw std::length_error("Vector is too long to write.");

    WriteInt(static_cast<uint32_t>(vec.size()));
    for (const auto& item : vec)
    {
      WriteValue(item);
    }
  }

  void WriteValue(bool value) { WriteBool(value); }

  template <std::integral T>
  void WriteValue(T value) { WriteInt(value); }

  void WriteValue(float value) { WriteFloat(value); }
  void WriteValue(double value) { WriteDouble(value); }
  void WriteValue(std::string_view string) { WriteString(string); }

  template <typename T>
      requires requires(BinaryWriter& w, const T& v) { Serialize(w, v); }
  void WriteValue(const T& value)
  {
      Serialize(*this, value);
  }

  void WriteBytes(std::span<const std::byte> bytes);

  [[nodiscard]] std::vector<std::byte> Build() const { return m_buffer; }

  [[nodiscard]] std::span<const std::byte> GetBuffer() const { return m_buffer; }

  [[nodiscard]] std::vector<std::byte> TakeBuffer()
  {
    return std::move(m_buffer);
  }

private:
  std::vector<std::byte> m_buffer;

};

inline void BinaryWriter::WriteBytes(std::span<const std::byte> bytes)
{
    m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
}
