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

int main(int argc, char** argv)
{   
    EventBase base(1000);
    TcpConnPtr con = TcpConn::createConnection(&base, "www.baidu.com", 80);
    HttpConnPtr http(con);
    http.onHttpMsg([con](const HttpConnPtr& cb)
    {
        cout << cb.getResponse().body << endl;
    });

    con->onState([&base, &http](const TcpConnPtr& c)
    {
        if(c->state_ == TcpConn::Closed)
        {
            if(c->reconnectInterval_ < 0)
            base.exit();
        }
        if(c->state_ == TcpConn::Connected)
        {
            cout << "connect" << endl;
            http.sendRequest();
        }
    });
    /*
    Buffer test;
    
    
    for(int i = 0; i< 10; i++){
    test.append("hello world...");
    con->send(test);}
    con->sendOutput();

    
    
    base.runAfter(3000,[con](){con->send("hello world,,,");},1000);
    */
    base.loop();
    return 0;
}
