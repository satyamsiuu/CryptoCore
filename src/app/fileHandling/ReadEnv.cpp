#include <iostream>
#include <string>
#include "IO.hpp"
#include <sstream>

class ReadEnv
{
public:
    std::string read()
    {
        std::string env_path = ".env";
        IO io(env_path);
        std::fstream f_stream = io.getFileStream();
        if (!f_stream.is_open())
            return std::string();
        std::stringstream buffer;
        buffer << f_stream.rdbuf();
        std::string content = buffer.str();
        return content;
    }
};
