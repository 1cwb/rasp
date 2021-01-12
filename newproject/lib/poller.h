#pragma once
#include <sys/epoll.h>
#include <assert.h>
#include <map>
#include <atomic>
#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include "util.h"
#include "event_base.h"

namespace rasp
{
    const int kMaxEvents = 2000;
    const int kReadEvent = EPOLLIN;
    const int kWriteEvent = EPOLLOUT;

    struct PollerBase : private noncopyable
    {
        int64_t id_;
        int lastActive_;
        PollerBase(): lastActive_(-1){static std::atomic<int64_t> id(0); id_ = ++id;}
        virtual void addChannel(Channel* ch) = 0;
        virtual void removeChannel(Channel* ch) = 0;
        virtual void updateChannel(Channel* ch) = 0;
        virtual void loop_once(int waitMs) = 0;
        virtual ~PollerBase(){}
    };
    PollerBase* createPoller();
}