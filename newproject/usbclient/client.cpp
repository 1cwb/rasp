#include <iostream>
#include "config.h"
#include "common.h"
#include "util.h"
#include <chrono>
#include "mlog.h"
#include "rasp_impl.h"
#include "event_base.h"
#include "conn.h"
using namespace std;
using namespace rasp;

int main(int argc, char** argv)
{   
    EventBase base(1000);
    TcpConnPtr con = TcpConn::createConnection(&base, "192.168.31.162", 4748);

    Buffer test;
    
    
    for(int i = 0; i< 10; i++){
    test.append("hello world...");
    con->send(test);}
    con->sendOutput();

    con->onState([&base](const TcpConnPtr& c)
    {
        if(c->state_ == TcpConn::Closed)
        {
            if(c->reconnectInterval_ < 0)
            base.exit();
        }
        if(c->state_ == TcpConn::Connected)
        {
            c->send("fuck every thing");
        }
    });
    con->onRead([](const TcpConnPtr& c){
        info("%s",c->getInput());
        //c->send(c->getInput());
        //c->sendOutput();
    });
    con->onWritable([](const TcpConnPtr& c){
        info("xxxxx can write\n");
    });
    con->addIdleCB(30, [](const TcpConnPtr& c){
        //c->close();
    });
    con->setReconnectInterval(5000);
    //base.runAfter(3000,[con](){con->send("hello world,,,");},1);
    base.loop();
    return 0;
}
