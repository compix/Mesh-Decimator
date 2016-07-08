#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace file
{
    std::string readAsString(const std::string& path);
    void loadRawBuffer(const std::string& path, std::vector<char>& outBuffer, uint32_t& outNumValues);

    bool exists(const std::string& filename);
    size_t getSize(const std::string& filename);
}
