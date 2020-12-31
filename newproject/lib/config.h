#pragma once
#include <map>
#include <list>
#include <string>
#include "common.h"

namespace rasp
{
    class Config
    {
    public:
        // 0 success
        // -1 IOERROR
        // >0 line no of error
        int32_t parse(const std::string& filename1);
        
        // Get an string value from ini file, return default value if not found.
        std::string get(std::string section, std::string name, std::string default_value);

        // Get a integer value from ini file, return default value if not fond
        long getInteger(std::string section, std::string name, long default_value);

        // Get a real (floating point double) value from ini file, return default 
        // value if not found
        double getReal(std::string section, std::string name, double default_value);

        // Get a boolean value from ini file, return default value if error
        bool getBoolean(std::string section, std::string name, bool default_value);

        // Get a string value from ini file, return empty list if not found
        std::string getStrings(std::string section, std::string name);

        // dump all value
        void dump();
    private:
        std::string makeKey(std::string section, std::string name);
        std::map<std::string, std::string> values_;
        std::string filename;
    };
};