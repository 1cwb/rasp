#include "threads.h"
#include <assert.h>
#include <utility>
#include <iostream>

using namespace std;

namespace rasp
{
    template class SafeQueue<Task>;

    ThreadPool::ThreadPool(int threads, int taskCapacity, bool start) :
    tasks_(taskCapacity), threads_(threads)
    {
        if(start)
        {
            this->start();
        }
    }
    ThreadPool::~ThreadPool()
    {
        assert(tasks_.exited());
        if(tasks_.size())
        {
            fprintf(stderr, "%lu tasks not processed when thread pool exited\n",tasks_.size());
        }
    }
    void ThreadPool::start()
    {
        for(auto& th : threads_)
        {
            thread t(
                [this](){
                    while(!tasks_.exited())
                    {
                        Task task;
                        if(tasks_.pop_wait(&task))
                        {
                            task();
                        }
                    }        
                }
            );
            th.swap(t);
        }
    }
    //ThreadPool& exit() {tasks_.exit(); return *this;}
    void ThreadPool::join()
    {
        for(auto& t : threads_)
        {
            t.join();
        }
    }
    bool ThreadPool::addTask(Task&& task)
    {
        return tasks_.push(move(task));
    }
    //bool ThreadPool::addTask(Task& task) {return addTask(Task(task));}
    //size_t ThreadPool::taskSize() {return tasks_.size();}
   // private:
     //   SafeQueue<Task> tasks_;
     //   std::vector<std::thread> threads_;
}