#pragma once

#include <filesystem>
#include <string>

struct TempFile
{
    std::filesystem::path path;

    explicit TempFile(std::string filename) : path(std::move(filename)) {}

    ~TempFile()
    {
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    std::string Str() const { return path.string(); }
};