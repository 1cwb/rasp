#include "poller.h"
#include <set>

namespace rasp
{
    struct PollerEpoll : public PollerBase
    {
        int fd_;
        std::set<Channel*>  liveChabbels_;
        PollerEpoll();
        virtual ~PollerEpoll();
        struct epoll_event activeEvs_[kMaxEvents]; 
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
        while(liveChabbels_.size())
        {
            (*liveChabbels_.begin())->close();
        }
        close(fd_);
        info("poller %d destroyed",fd_);
    }
    void PollerEpoll::addChannel(Channel* ch) 
    {

    }
    void PollerEpoll::removeChannel(Channel* ch) 
    {

    }
    void PollerEpoll::updateChannel(Channel* ch) 
    {

    }
    void PollerEpoll::loop_once(int waitMs) 
    {

    }

    PollerBase* createPoller()
    {
        return new PollerEpoll();
    }
}