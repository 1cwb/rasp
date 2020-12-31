#include "config.h"
#include <iostream>
#include <algorithm>
#include <memory>
#include <fstream>
#include <stdlib.h>
#include <regex>

using namespace std;

namespace rasp 
{
    string Config::makeKey(string section, string name)
    {
        string key =  section + '.' + name;
        transform(key.begin(), key.end(), key.begin(), ::tolower);
        return key;
    }

    int32_t Config::parse(const string& filename1)
    {
        filename = filename1;
        regex regComment("^ *#+");
        regex regSection("^ *\\[.+\\]");
        smatch sm;
        ifstream file(filename, ios::in);
        if(!file)
        {
            return -1;
        }
        static const int32_t MAX_LINE = 16 * 1024;
        char* ln = new char[MAX_LINE];
        unique_ptr<char[]> release1(ln);
        string linestr;
        int32_t lineno = 0;
        string section, name, valuestr;
        while(!file.eof())
        {
            memset(ln, 0, MAX_LINE);
            file.getline(ln, MAX_LINE);
            lineno ++;
            linestr = ln;
            if(regex_search(linestr, sm, regComment))
            {
                //get #
                continue;
            }
            else if(regex_search(linestr, sm, regSection))
            {
                section = sm.str();
                section.erase(remove(section.begin(), section.end(), ' '), section.end());
                section.erase(remove(section.begin(), section.end(), '['), section.end());
                section.erase(remove(section.begin(), section.end(), ']'), section.end());
                name = "";
            }
            else if(linestr.find('=', 0) != string::npos)
            {
                string::size_type pos = linestr.find('#', 0);
                if(pos != string::npos)
                {
                    linestr.erase(pos, string::npos);
                }
                pos = linestr.find('=', 0);
                if(pos == string::npos)
                {
                    continue;
                }
                name = linestr.substr(0, pos);
                if(name.empty())
                {
                    continue;
                }
                name.erase(remove(name.begin(), name.end(), ' '), name.end());
                valuestr = linestr.substr(pos + 1, string::npos);
                if(valuestr.empty())
                {
                    continue;
                }
                valuestr.erase(0, valuestr.find_first_not_of(' '));
                valuestr.erase(valuestr.find_last_not_of(' ') + 1);
                pair<map<string, string>::iterator, bool> res = 
                    values_.insert(map<string, string>::value_type(makeKey(section, name), valuestr));
                if(!res.second)
                {
                    continue;
                }
            }
        }
        file.close();
        return 0;
    }

    string Config::get(string section, string name, string default_value)
    {
        string key = makeKey(section, name);
        auto p = values_.find(key);
        return p == values_.end() ? default_value : p->second;
    }

    long Config::getInteger(string section, string name, long default_value)
    {
        string valstr = get(section, name, "");
        const char* value = valstr.c_str();
        char* end;
        long n = strtol(value, &end, 0);
        return end > value ? n : default_value;
    }

    double Config::getReal(string section, string name, double default_value)
    {
        string valstr = get(section, name, "");
        const char* value = valstr.c_str();
        char* end;
        double n = strtod(value, &end);
        return end > value ? n: default_value;
    }

    bool Config::getBoolean(string section, string name, bool default_value)
    {
        string valstr = get(section, name, "");
        transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
        if(valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
        {
            return true;
        }
        else if(valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
        {
            return false;
        }
        return default_value;
    }

    string Config::getStrings(string section, string name)
    {
        return get(section, name, "");
    }

    void Config::dump()
    {
        for(auto it : values_)
        {
            cout <<it.first<<"="<<it.second<< endl;
        }
    }
};
       