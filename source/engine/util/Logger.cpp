#include "Logger.h"

Logger::Logger()
{

}

void Logger::log(const std::string& msg)
{
    stream() << msg << std::endl;
}