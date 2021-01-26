#pragma once
#include "mlog.h"
#include "util.h"
#include "net.h"
#include "threads.h"
#include "codec.h"
#include <utility>
#include <set>
#include <memory>
#include <unistd.h>

namespace rasp
{
    struct PollerBase;
    struct Channel;
    struct TcpConn;
    struct TcpServer;
    struct IdleIdImp;
    struct EventsImp;
    struct EventBase;
    typedef std::unique_ptr<IdleIdImp> IdleId;
    typedef std::pair<int64_t, int64_t> TimerId;

    struct AutoContext : private noncopyable
    {
        void* ctx;
        Task ctxDel;
        AutoContext():ctx(nullptr) {}
        template<typename T>
        T& context() 
        {
            if(ctx == nullptr)
            {
                ctx = new T();
                ctxDel = [this] () {delete (T*)ctx;};
            }
            return *(T*) ctx;
        }
        ~AutoContext() {if(ctx) ctxDel();}
    };
}