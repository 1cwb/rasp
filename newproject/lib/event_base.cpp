#include "event_base.h"
#include <string.h>
#include <fcntl.h>
#include <list>
#include <thread>
#include "threads.h"
//#include "conn.h"

using namespace std;

namespace rasp
{
    struct TimerRepeatable 
    {
        int64_t at;
        int64_t interval;
        TimerId timerid;
        Task    cb;
    };

    struct IdleNode
    {
        TcpConnPtr con_;
        int64_t updated_;
        TcpCallBack cb_;
    };

    struct IdleIdImp
    {
        IdleIdImp(){}
        typedef list<IdleNode>::iterator Iter;
        IdleIdImp(list<IdleNode>* lst, Iter iter): lst_(lst), iter_(iter){}
        list<IdleNode>* lst_;
        Iter iter_;
    };
    struct EventsImp
    {
        EventBase* base_;
        PollerBase* poller_;
        std::atomic<bool> exit_;
        int wakeupFds_[2];
        int nextTimeout_;
        SafeQueue<Task> tasks_;

        std::map<TimerId, TimerRepeatable> timerReps_;
        std::map<TimerId, Task> timers_;
        std::atomic<int64_t> timerSeq_;
        std::map<int, std::list<IdleNode>> idleConns_;
        std::set<TcpConnPtr> reconnectConns_;
        bool idleEnabled;

        EventsImp(EventBase* base, int taskCap);
        ~EventsImp();
        void init();
        void callIdles();
        IdleId registerIdle(int idle, const TcpConnPtr& con, const TcpCallBack& cb);
        void unregisterIdle(const IdleId& id);
        void updateIdle(const IdleId& id);
        void handleTimeouts();
        void refreshNearest(const TimerId* tid = nullptr);
        void repeatableTimeout(TimerRepeatable* tr);

        EventBase& exit() {exit_ = true; wakeup(); return *base_;}
        bool exited() {return exit_;}
        void safeCall(Task&& task) {tasks_.push(move(task)); wakeup();}
        void loop();
        void loop_once(int waitMs) {poller_->loop_once(std::min(waitMs, nextTimeout_)); handleTimeouts();}
        void wakeup() 
        {
            int r = write(wakeupFds_[1], "", 1);
            fatalif(r <= 0, "write error wd %d %d %s",r, errno, strerror(errno));
        }
        bool cancel(TimerId timerid);
        TimerId runAt(int64_t milli, Task&& task, int64_t interval);
    };
    
    EventBase::EventBase(int taskCapacity)
    {
        imp_.reset(new EventsImp(this, taskCapacity));
        imp_->init();
    }
    EventBase::~EventBase()
    {

    }
    void EventBase::loop_once(int waitMs)
    {
        imp_->loop_once(waitMs);
    }
    void EventBase::loop()
    {
        imp_->loop();
    }
    bool EventBase::cancel(TimerId timerid)
    {
        return imp_ && imp_->cancel(timerid);
    }
    TimerId EventBase::runAt(int64_t milli, Task&& task, int64_t interval)
    {
        return imp_->runAt(milli, std::move(task), interval);
    }
    EventBase& EventBase::exit()
    {
        return imp_->exit();
    }
    bool EventBase::exited()
    {
        return imp_->exited();
    }
    void EventBase::wakeup()
    {
        imp_->wakeup();
    }
    void EventBase::safeCall(Task&& task)
    {
        imp_->safeCall(move(task));
    }
//==================impl========================
    EventsImp::EventsImp(EventBase* base, int taskCap): 
        base_(base), poller_(createPoller()), exit_(false), nextTimeout_(1 << 30), 
        tasks_(taskCap), timerSeq_(0), idleEnabled(false)
    {
    
    }
    EventsImp::~EventsImp()
    {
        delete poller_;
        close(wakeupFds_[1]);
    }
    void EventsImp::init()
    {
        int r = pipe(wakeupFds_);
        fatalif(r, "pipe failed %d %s", errno, strerror(errno));
        r = util::addFdFlag(wakeupFds_[0], FD_CLOEXEC);
        fatalif(r, "addFdFlag failed %d %s", errno, strerror(errno));
        r = util::addFdFlag(wakeupFds_[1], FD_CLOEXEC);
        fatalif(r, "addFdFlag failed %d %s", errno, strerror(errno));
        trace("wakeup pipe created %d %d", wakeupFds_[0], wakeupFds_[1]);
        Channel* ch = new Channel(base_, wakeupFds_[0], kReadEvent);
        ch->onRead([=](){
            char buf[1024];
            int r = ch->fd() >= 0 ? read(ch->fd(), buf, sizeof(buf)) : 0;
            if(r > 0)
            {
                Task task;
                while(tasks_.pop_wait(&task, 0))
                {
                    task();
                }
            }
            else if(r == 0)
            {
                delete ch;
            }
            else if(errno == EINTR)
            {

            }
            else
            {
                fatal("wakeup channel read error %d %d %s", r, errno, strerror(errno));
            }
        });
    }
    void EventsImp::callIdles()
    {
        int64_t now = util::timeMilli() / 1000;
        for(auto& l : idleConns_)
        {
            int idle = l.first;
            auto lst = l.second;
            while(lst.size())
            {
                IdleNode& node = lst.front();
                if(node.updated_ + idle > now) 
                {
                    break;
                }
                node.updated_ = now;
                lst.splice(lst.end(), lst, lst.begin());
            }
        }
    }
    IdleId EventsImp::registerIdle(int idle, const TcpConnPtr& con, const TcpCallBack& cb)
    {
        if(!idleEnabled)
        {
            base_->runAfter(1000, [this](){callIdles();}, 1000);
            idleEnabled = true;
        }
        auto& lst = idleConns_[idle];
        lst.push_back(IdleNode {con, util::timeMilli() / 1000, move(cb)});
        trace("register idle");
        return IdleId(new IdleIdImp(&lst, --lst.end()));
    }
    void EventsImp::unregisterIdle(const IdleId& id)
    {
        trace("unregister idle");
        id->lst_->erase(id->iter_);
    }
    void EventsImp::updateIdle(const IdleId& id)
    {
        trace("update idle");
        id->iter_->updated_ = util::timeMilli() / 1000;
        id->lst_->splice(id->lst_->end(), *id->lst_, id->iter_);
    }
    void EventsImp::handleTimeouts()
    {
        int64_t now = util::timeMilli();
        TimerId tid {now, 1L << 62};
        while (timers_.size() && timers_.begin()->first < tid)
        {
            Task task = move(timers_.begin()->second);
            timers_.erase(timers_.begin());
            task();
        }
        refreshNearest();
    }
    void EventsImp::refreshNearest(const TimerId* tid)
    {
        if(timers_.empty())
        {
            nextTimeout_ = 1 << 30;
        }
        else
        {
            const TimerId& t = timers_.begin()->first;
            nextTimeout_ = t.first - util::timeMilli();
            nextTimeout_ = nextTimeout_ < 0 ? 0 : nextTimeout_;
        }
    }
    void EventsImp::repeatableTimeout(TimerRepeatable* tr)
    {
        tr->at += tr->interval;
        tr->timerid = {tr->at, ++timerSeq_};
        timers_[tr->timerid] = [this, tr](){repeatableTimeout(tr);};
        refreshNearest(&tr->timerid);
        tr->cb();
    }
    void EventsImp::loop()
    {
        while(!exit_)
        {
            loop_once(10000);
            timerReps_.clear();
            timers_.clear();
            idleConns_.clear();
            for(auto recon : reconnectConns_)
            {
                recon->cleanup(recon);
            }
            loop_once(0);
        }
    }
    bool EventsImp::cancel(TimerId timerid)
    {
       if(timerid.first < 0)
       {
           auto p = timerReps_.find(timerid);
           auto ptimer = timers_.find(p->second.timerid);
           if(ptimer != timers_.end())
           {
               timers_.erase(ptimer);
           }
           timerReps_.erase(p);
           return true;
       }
       else
       {
           auto p = timers_.find(timerid);
           if(p != timers_.end())
           {
               timers_.erase(p);
               return true;
           }
       }
       return false;
    }
    TimerId EventsImp::runAt(int64_t milli, Task&& task, int64_t interval)
    {
        if(exit_)
        {
            return TimerId();
        }
        if(interval)
        {
            TimerId tid {-milli, ++timerSeq_};
            TimerRepeatable& rtr = timerReps_[tid];
            rtr = {milli, interval, {milli, ++timerSeq_}, move(task)};
            TimerRepeatable* tr = &rtr;
            timers_[tr->timerid] = [this, tr](){repeatableTimeout(tr);};
            refreshNearest(&tr->timerid);
            return tid;
        }
        else
        {
            TimerId tid {milli, ++timerSeq_};
            timers_.insert({tid, move(task)});
            refreshNearest(&tid);
            return tid;
        }
    }

    //MultiBase
    void MultiBase::loop()
    {
        int sz = bases_.size();
        vector<thread> ths(sz - 1);
        for(int i = 0; i < sz - 1; i++)
        {
            thread t([this, i](){bases_[i].loop();});//run all base loop()
            ths[i].swap(t);
        }
        bases_.back().loop();//(size - 1)
        for(int i = 0; i < sz - 1; i++)
        {
            ths[i].join();
        }
    }

    //Channel
    Channel::Channel(EventBase* base, int fd, int events): base_(base), fd_(fd), events_(events)
    {
        fatalif(net::setNonBlock(fd_) < 0, "channel set non block failed");
        static atomic<int64_t> id(0);
        id_ = ++id;
        poller_ = base_->imp_->poller_;
        poller_->addChannel(this);
    }
    Channel::~Channel()
    {
        this->close();
    }
    void Channel::close()
    {
        if(fd_ >= 0)
        {
            trace("close channel %ld fd %d", (long)id_, fd_);
            poller_->removeChannel(this);
            ::close(fd_);
            fd_ = -1;
            handleRead();
        }
    }
    void Channel::enableRead(bool enable)
    {
        if(enable)
        {
            events_ |= kReadEvent;
        }
        else
        {
            events_ &= ~kReadEvent;
        }
        poller_->updateChannel(this);
    }
    void Channel::enableWrite(bool enable)
    {
        if(enable)
        {
            events_ |= kWriteEvent; 
        }
        else
        {
            events_ &= ~kWriteEvent;
        }
        poller_->updateChannel(this);
    }
    void Channel::enableReadWrite(bool readable, bool writable)
    {
        if(readable)
        {
            events_ |= kReadEvent;
        }
        else
        {
            events_ &= ~kReadEvent;
        }
        if(writable)
        {
            events_ |= kWriteEvent; 
        }
        else
        {
            events_ &= ~kWriteEvent;
        }
        poller_->updateChannel(this);
    }
    bool Channel::readEnabled()
    {
        return events_ & kReadEvent;
    }
    bool Channel::writeEnabled()
    {
        return events_ & kWriteEvent;
    }
    void raspUnregisterIdle(EventBase* base, const IdleId& idle)
    {
        base->imp_->unregisterIdle(idle);
    }
    void raspUpdateIdle(EventBase* base, const IdleId& idle)
    {
        base->imp_->updateIdle(idle);
    }
    //EventBase* base_;
    //PollerBase* poller_;
    //int fd_;
    //short events_;
    //int64_t id_;
    //std::function<void()> readcb_, writecb_, errorcb_;
}