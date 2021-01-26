#include "util.h"
#include <iostream>
#include <memory>
#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

namespace rasp
{
    std::string util::format(const char* fmt, ...)
    {
        char buffer[500];
        unique_ptr<char []> release1;
        char* base = nullptr;
        for(int iter = 0; iter < 2; iter ++)
        {
            int buffsize;
            if(iter == 0)
            {
                buffsize = sizeof(buffer);
                base = buffer;
            }
            else
            {
                buffsize = 30000;
                base = new char[buffsize];
                release1.reset(base);
            }
            char* p = base;
            char* limit = base + buffsize;
            if(p < limit)
            {
                va_list ap;
                va_start(ap, fmt);
                p += vsnprintf(p, limit - p, fmt, ap);
                va_end(ap);
            }
            if(p >= limit)
            {
                if(iter == 0)
                {
                    continue;
                }
                else
                {
                    p = limit - 1;
                    *p = '\0';
                }
                break;
            }
        }
        return base;
    }
    int64_t util::timeMicro()
    {
        chrono::time_point<chrono::system_clock> p = chrono::system_clock::now();
        return chrono::duration_cast<chrono::microseconds>(p.time_since_epoch()).count();
    }
    int64_t util::steadyMicro()
    {
        chrono::time_point<chrono::steady_clock> p = chrono::steady_clock::now();
        return chrono::duration_cast<chrono::microseconds>(p.time_since_epoch()).count();
    }
    std::string util::readableTime(time_t t)
    {
        struct tm tm1;
        localtime_r(&t, &tm1);
        return format("%04d-%02d-%02d %02d:%02d:%02d",
            tm1.tm_year + 1900, tm1.tm_mon, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
    }
    int util::addFdFlag(int fd, int flag)
    {
        int ret = fcntl(fd, F_GETFD);
        return fcntl(fd, F_SETFD, ret | flag);
    }
     std::string util::runUnixCommand(const std::string cmd, const std::string type)
    {
        char result[2048];
        string sresult;
        memset(result, 0, sizeof(result));
        FILE* fp = popen(cmd.c_str(), type.c_str());
        if(fp == nullptr)
        {
            return "Error fail popen " + cmd;
        }
        ExitCaller eclosefd([&fp]()->void{if(fp != nullptr){
            pclose(fp); fp = nullptr;
        }});
        while(fgets(result, sizeof(result), fp) != nullptr)
        {
            sresult.append(result, sizeof(result));
            memset(result, 0, sizeof(result));
        }
        return sresult;
    }
}
