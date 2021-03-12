#include "status.h"

namespace rasp
{
    const char* Status::copyState(const char* state)
    {
        if(state == nullptr)
        {
            return state;
        }
        uint32_t size = *(uint32_t*) state;
        char* res = new char[size];
        memcpy(res, state, size);
        return res;
    }
    Status::Status(int code, const char* msg)
    {
        int size = 8 + strlen(msg);
        char* p = new char[size];
        state_ = p;
        *(uint32_t*) p = size;
        *(uint32_t*) (p + 4) = code;
        memcpy(p+8, msg, size - 8);
    }
    #include <iostream>
    Status Status::fromFormat(int code, const char* fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        uint32_t size = 8 + vsnprintf(nullptr, 0, fmt, ap) + 1;
        va_end(ap);
        Status r;
        r.state_ = new char[size];
        *(uint32_t *) r.state_ = size;
        *(int32_t *) (r.state_ + 4) = code;
        va_start(ap, fmt);
        vsnprintf((char *) r.state_ + 8, size - 8, fmt, ap);
        va_end(ap);
        return r;
    }
}