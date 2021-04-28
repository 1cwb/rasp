#include <iostream>
#include <chrono>
#include "config.h"
#include "common.h"
#include "util.h"
#include "file.h"
#include "mlog.h"
#include "rasp_impl.h"
#include "event_base.h"
#include "conn.h"
#include "http.h"
using namespace std;
using namespace rasp;
#define DBG_CLIENT 1

enum ParseState
{
    E_HOMEPAGE,
    E_NORMALPAGE
}; 
using E_PARSE_STATE = ParseState;

int main(int argc, char** argv)
{   
    EventBase base;
    E_PARSE_STATE state = E_HOMEPAGE;
    TcpConnPtr con = TcpConn::createConnection(&base, "m2.5y1rsxmzh.club",80);
    cout << "IP is" <<con->peer_.toString() << endl;
    HttpConnPtr http(con);
    http->setReconnectInterval(20000);
    http->onState([&http](const TcpConnPtr& con){
        if(con->getState() == TcpConn::Connected)
        {
            cout << "connect to " + con->destHost_ + " sucessful"<<endl;
            http.getRequest().getMethod() = "GET";
            http.getRequest().getQureUri() = "/pw/";
            http.getRequest().getHeaders()["Host"] = "m2.5y1rsxmzh.club";
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
        if(state == E_HOMEPAGE)
        {
            state = E_HOMEPAGE;
            File::writeContent("download.html",m.getResponse().getBody());
        }

        
        
        /*base.runAfter(5, [&](){
            http.getRequest().getMethod() = "GET";
            http.getRequest().getQureUri() = "/pw/";
            http.getRequest().getHeaders()["Host"] = "m2.5y1rsxmzh.club";
            http.getRequest().getHeaders()["Accept-Language"] = "cn";  
            http.getRequest().getHeaders()["User-Agent"] = "Mozilla/5.0";  
            http.getRequest().getHeaders()["Cache-control"] = "no-cache";  
            http.sendRequest();  
        });*/
        cout << "Response:===========resent=================" << endl;;
    });
    http->addIdleCB(10, [&](const TcpConnPtr& con){
        if(con->getState() != TcpConn::Connected)
        {
            cout << "now connect is not enabled!!!!, wait for may second"<<endl;
            return;
        }
        cout << "call cb is runll->>>>"<<endl;
         http.getRequest().getMethod() = "GET";
        http.getRequest().getQureUri() = "/pw/";
        http.getRequest().getHeaders()["Host"] = "m2.5y1rsxmzh.club";
        http.getRequest().getHeaders()["Accept"] = "*/*";
        http.getRequest().getHeaders()["Accept-Language"] = "cn";  
        http.getRequest().getHeaders()["User-Agent"] = "Mozilla/5.0";  
        http.getRequest().getHeaders()["Cache-control"] = "no-cache";  
        http.sendRequest();
    });
    base.loop();
    return 0;
}
