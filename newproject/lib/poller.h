#pragma once
#include <poll.h>
#include <assert.h>
#include <map>
#include <atomic>
#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include "util.h"

namespace rasp
{
    const int kMaxEvents = 2000;
    const int kReadEvent = POLLIN;
    const int kWriteEvent = POLLOUT;

    struct PollerBase : private noncopyable
    {
        int64_t id_;
        int lastActive_;
        PollerBase(): lastActive_(-1){static std::atomic<int64_t> id(0); id_ = ++id;}
    };
    
}