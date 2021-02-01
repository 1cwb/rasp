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
    con->onRead([](const TcpConnPtr& c){
        info("get msg : %s",c->getInput().data());
    });
    con->send("hello world,,,");
    //base.runAfter(3000,[con](){con->send("hello world,,,");},1);
    base.loop();
    return 0;
}
