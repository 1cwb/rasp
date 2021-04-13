#include <iostream>
#include "config.h"
#include "common.h"
#include "util.h"
#include <chrono>
#include "mlog.h"
#include "rasp_impl.h"
#include "event_base.h"
#include "conn.h"
#include "http.h"
using namespace std;
using namespace rasp;
#define DBG_CLIENT 1
//https://news.google.com/topstories?hl=zh-TW&gl=TW&ceid=TW:zh-Hant
int main(int argc, char** argv)
{   
    EventBase base;
    TcpConnPtr con = TcpConn::createConnection(&base, "www.360.cn",80);
    cout << "IP is" <<con->peer_.toString() << endl;
    //TcpConnPtr con = TcpConn::createConnection(&base, "www.baidu.com", 80);
    HttpConnPtr http(con);
    http->onState([&http](const TcpConnPtr& con){
        if(con->getState() == TcpConn::Connected)
        {
            http.getRequest().getMethod() = "GET";
            http.getRequest().getQureUri() = "/";
            http.getRequest().getHeaders()["Host"] = "www.360.cn";
            http.getRequest().getHeaders()["Accept"] = "*/*";
            http.getRequest().getHeaders()["Accept-Language"] = "cn";  
            http.getRequest().getHeaders()["User-Agent"] = "Mozilla/5.0";  
            http.getRequest().getHeaders()["Cache-control"] = "no-cache";  
            http.sendRequest();
        }
    });
    http.onHttpMsg([&](const HttpConnPtr& m){
        #if DBG_CLIENT
        cout << "Response:===============begin==================" << endl;;
        cout << m.getResponse().getStatus() <<" " << m.getResponse().getStatusWord() << endl;
        for(auto& t : m.getResponse().getHeaders())
        {
            cout << t.first << ": " << t.second << endl;
        }
        cout << m.getResponse().getBody().toString() << endl;
        cout << "Response:=============end====================" << endl;;
        #endif
        base.runAfter(5000, [&](){
            http.getRequest().getMethod() = "GET";
            http.getRequest().getQureUri() = "/";
            http.getRequest().getHeaders()["Host"] = "www.360.cn";
            http.getRequest().getHeaders()["Accept"] = "*/*";
            http.getRequest().getHeaders()["Accept-Language"] = "cn";  
            http.getRequest().getHeaders()["User-Agent"] = "Mozilla/5.0";  
            http.getRequest().getHeaders()["Cache-control"] = "no-cache";  
            http.sendRequest();
        });
        cout << "Response:===========resent=================" << endl;;
    });
    base.loop();
    return 0;
}
