#include "../FileIO.h"

#include <fstream>

std::vector<std::byte> LoadFile(const std::string &path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        throw std::runtime_error("Failed to open file: " + path);
    auto size = file.tellg();
    if (size < 0)
        throw std::runtime_error("Failed to get file size: " + path);

    file.seekg(0, std::ios::beg);

    std::vector<std::byte> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size))
        throw std::runtime_error("Failed to read file: " + path);
    return buffer;
}

void SaveFile(const std::string &path, std::span<const std::byte> data)
{
    std::ofstream file(path, std::ios::binary );
    if (!file)
        throw std::runtime_error("Failed to open file for writing: " + path);
    if (!file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size())))
        throw std::runtime_error("Failed to write data to file: " + path);
}
