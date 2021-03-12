#pragma once
#include <errno.h>
#include <stdarg.h>
#include <cstring>
#include "util.h"

namespace rasp
{
    inline const char* errstr()
    {
        return strerror(errno);
    }
    struct Status
    {
        Status() : state_(nullptr) {}
        Status(int code, const char* msg);
        Status(int code, const std::string& msg) : Status(code, msg.c_str()){}
        Status(const Status& s)
        {
            state_ = copyState(s.state_);
        }
        void operator=(const Status& s)
        {
            if(state_)
            {
                delete[] state_;
            }
            state_ = copyState(s.state_);
        }
        Status(Status&& s)
        {
            //state_ may not be nullptr
            state_ = s.state_;
            s.state_ = nullptr;
        }
        void operator=(Status&& s)
        {
            if(state_)
            {
                delete[] state_;
            }
            state_ = s.state_;
            s.state_ = nullptr;
        }
        ~Status() {if(state_){delete[] state_;}}

        static Status fromSystem() {return Status(errno, strerror(errno));}
        static Status fromSystem(int err) { return Status(err, strerror(err));}
        static Status fromFormat(int code, const char* fmt, ...);
        static Status ioError(const std::string& op, const std::string& name)
        {
            return Status::fromFormat(errno, "%s %s %s", op.c_str(), name.c_str(), errstr());
        }

        int code() {return state_ ? *(uint32_t*) (state_ + 4) : 0;}
        const char* msg() {return state_ ? state_ + 8 : "";}
        bool ok() {return code() == 0;}
        std::string toString() { return util::format("%d %s", code(), msg());}
    private:
        //state_[0..3] == length of message
        //state_[4..7] == code
        //state_[8...] == message
        const char* state_;
        const char* copyState(const char* state);
    };
}