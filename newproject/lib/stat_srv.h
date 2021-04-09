#pragma once

#include "slice.h"
#include "event_base.h"
#include "http.h"
#include <map>
#include <functional>

namespace rasp
{
    typedef std::function<void(const HttpRequest&, HttpResponse&)> StatCallBack;
    typedef std::function<std::string()> InfoCallBack;
    typedef std::function<int64_t()> IntCallBack;

    struct StatServer: private noncopyable
    {
        enum StatType {STATE, PAGE, CMD};
        StatServer(EventBase* base);
        int bind(const std::string& host, short port){return server_.bind(host, port);}


    private:
        HttpServer server_;
        typedef std::pair<std::string, StatCallBack> DescState;
        std::map<std::string, DescState> statcbs_, pagecbs_, cmdcbs_;
        std::map<std::string, StatCallBack> allcbs_;
    };
}