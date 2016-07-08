#include "file.h"
#include <fstream>
#include "Logger.h"
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

std::string file::readAsString(const std::string& path)
{
    std::string fileAsString = "";
    std::ifstream stream(path, std::ios::in);

    if (!stream.is_open())
    {
        Logger::stream() << "Could not open file: " << path << std::endl;
        return "";
    }

    std::string line = "";
    while (getline(stream, line))
        fileAsString += line + "\n";

    stream.close();

    return fileAsString;
}

void file::loadRawBuffer(const std::string& path, std::vector<char>& outBuffer, uint32_t& outNumValues)
{
    std::ifstream input(path, std::ios::binary);
    outBuffer = { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
    outNumValues = *reinterpret_cast<uint32_t*>(&outBuffer[0]);
}

bool file::exists(const std::string& filename)
{
    struct stat buffer;
    return stat(filename.c_str(), &buffer) == 0;
}

size_t file::getSize(const std::string& filename)
{
    struct stat buffer;
    return stat(filename.c_str(), &buffer) == 0 ? buffer.st_size : 0;
}
