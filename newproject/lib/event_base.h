#pragma once
#include <iostream>
#include "rasp_impl.h"
#include "threads.h"
#include "poller.h"

namespace rasp
{
    struct PollerBase;
    typedef std::shared_ptr<TcpConn> TcpConnPtr;
    typedef std::shared_ptr<TcpServer> TcpServerPtr;
    typedef std::function<void(const TcpConnPtr&)> TcpCallBack;
   // typedef std::function<void(const TcpConnPtr&), Slice msg> MsgCallBack;

    struct EventBases : private noncopyable
    {
        virtual EventBase* allocBase() = 0;
    };

    struct EventBase : public EventBases
    {
        /*taskCapacity set task equeue size*/
        EventBase(int taskCapacity = 0);
        ~EventBase();
        /*deal with events on time*/
        void loop_once(int waitMs);
        /*for loop*/
        void loop();
        bool cancel(TimerId timerid);

        TimerId runAt(int64_t milli, const Task& task, int64_t interval = 0){return runAt(milli, Task(task), interval);}
        TimerId runAt(int64_t milli, Task&& Task, int64_t interval = 0);
        TimerId runAfter(int64_t milli, const Task& task, int64_t interval = 0){return runAt(util::timeMilli() + milli,Task(task), interval);}
        TimerId runAfter(int64_t milli, Task&& task, int64_t interval = 0){return runAt(util::timeMilli() + milli, std::move(task), interval);}

        //thread safe function
        EventBase& exit();
        bool exited();
        void wakeup();
        void safeCall(Task&& task);
        void safeCall(const Task& task) {safeCall(Task(task));}
        virtual EventBase* allocBase() {return this;}

    public:
        std::unique_ptr<EventsImp> imp_;
    };

    struct MultiBase : public EventBases
    {
        MultiBase(int sz): id_(0), bases_(sz){}
        virtual EventBase* allocBase() {int c = id_ ++; return &bases_[c % bases_.size()];}
        void loop();
        MultiBase& exit(){for(auto& b : bases_){b.exit();} return *this;}
    private:
        std::atomic<int> id_;
        std::vector<EventBase> bases_;
    };

    struct Channel : private noncopyable
    {
        Channel(EventBase* base, int fd, int events);
        ~Channel();
        EventBase* getBase(){return base_;}
        int fd() {return fd_;}

        int64_t id() {return id_;}
        short events() {return events_;}

        void close();

        void onRead(const Task& readcb) {readcb_ = readcb;}
        void onWrite(const Task& writecb) {writecb_ = writecb;}
        void onRead(Task&& readcb) {readcb_ = std::move(readcb);}
        void onWrite(Task&& writecb) {writecb_ = std::move(writecb);}

        void enableRead(bool enable);
        void enableWrite(bool enable);
        void enableReadWrite(bool readable, bool writable);
        bool readEnabled();
        bool writeEnabled();

        void handleRead() {readcb_();}
        void handleWrite() {writecb_();}

    protected:
        EventBase* base_;
        PollerBase* poller_;
        int fd_;
        short events_;
        int64_t id_;
        std::function<void()> readcb_, writecb_, errorcb_;
    };
}