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
    EventBase base;
    TcpServerPtr svr = TcpServer::startServer(&base, "", 99);
    svr->onConnRead([](const TcpConnPtr con){
        info("%s",con->getInput());
        con->send(con->getInput());
    });
    base.runAfter(3000,[](){static int i = 0; cout << i++<< endl;},1000);
    base.loop();
    return 0;
}
