#include "poller.h"
#include "mlog.h"
#include <unistd.h>
#include <set>
#include "event_base.h"

namespace rasp
{
    struct PollerEpoll : public PollerBase
    {
        int fd_;
        std::set<Channel*>  liveChannels_;
        struct epoll_event activeEvs_[kMaxEvents]; 
        PollerEpoll();
        virtual ~PollerEpoll();
        virtual void addChannel(Channel* ch) override;
        virtual void removeChannel(Channel* ch) override;
        virtual void updateChannel(Channel* ch) override;
        virtual void loop_once(int waitMs) override;
    };
    PollerEpoll::PollerEpoll()
    {
        fd_ = epoll_create1(EPOLL_CLOEXEC);
        fatalif(fd_ < 0, "epoll_create error %d %s", errno, strerror(errno));
        info("poller epoll %d created", fd_);
    }
    PollerEpoll::~PollerEpoll()
    {
        info("destroying poller %d", fd_);
        while(liveChannels_.size())
        {
            //Channel close() will call poller_.removeChannel(this);
            (*liveChannels_.begin())->close();
        }
        ::close(fd_);
        info("poller %d destroyed",fd_);
    }
    void PollerEpoll::addChannel(Channel* ch) 
    {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = ch->events();
        ev.data.ptr = ch;
        trace("adding channel %lld fd %d events %d epoll %d",(long long)ch->id(), ch->fd(), ev.events, fd_);
        int r = epoll_ctl(fd_, EPOLL_CTL_ADD, ch->fd(), &ev);
        fatalif(r, "epoll_ctl add failed %d %s",errno, strerror(errno));
        liveChannels_.insert(ch);
    }
    void PollerEpoll::removeChannel(Channel* ch) 
    {
        trace("deleting channel %lld fd %d epoll %d", (long long)ch->id(), ch->fd(), fd_);
        int r = epoll_ctl(fd_, EPOLL_CTL_DEL, ch->fd(), nullptr);
        fatalif(r, "epoll_ctl add failed %d %s",errno, strerror(errno));
        liveChannels_.erase(ch);
        for(int i = lastActive_; i >= 0; i --)
        {
            if(ch == activeEvs_[i].data.ptr)
            {
                activeEvs_[i].data.ptr = nullptr;
                break;
            }
        }

    }
    void PollerEpoll::updateChannel(Channel* ch) 
    {
        struct epoll_event ev;
        memset(&ev, 0, sizeof(ev));
        ev.events = ch->events();
        ev.data.ptr = ch;
        trace("modify channel %lld fd %d events read %d write %d epoll %d",
            (long long)ch->id(), ch->fd(), ev.events & EPOLLIN, ev.events & EPOLLOUT, fd_);
        int r = epoll_ctl(fd_, EPOLL_CTL_MOD, ch->fd(), &ev);
        fatalif(r, "epoll_ctl mod failed %d %s", errno, strerror(errno));
    }
    void PollerEpoll::loop_once(int waitMs) 
    {
        int64_t ticks = util::timeMilli();
        lastActive_ = epoll_wait(fd_, activeEvs_, kMaxEvents, waitMs);
        int64_t used = util::timeMilli() - ticks;
        trace("epoll wait %d return %d errno %d used %lld millsecond",
            waitMs, lastActive_, errno, (long long)used);
        fatalif(lastActive_ < 0 && errno != EINTR, "epoll return error %d, %s", errno, strerror(errno));
        while(-- lastActive_ >= 0)
        {
            int i = lastActive_;
            Channel* ch = reinterpret_cast<Channel*>(activeEvs_[i].data.ptr);
            int events = activeEvs_[i].events;
            if(ch)
            {
                if(events & (kReadEvent | EPOLLERR))
                {
                    trace("channel %lld fd %d handle read", (long long)ch->id(), ch->fd());
                    ch->handleRead();
                }
                else if(events & kWriteEvent)
                {
                    trace("channel %lld fd %d handle write", (long long)ch->id(), ch->fd());
                    ch->handleWrite();
                }
                else
                {
                    fatal("unexpected poller events");
                }
            }
        }
    }
    PollerBase* createPoller()
    {
        return new PollerEpoll;
    }
}