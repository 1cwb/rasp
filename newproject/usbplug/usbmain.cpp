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
     TcpServerPtr svr = TcpServer::startServer(&base, "192.168.31.162", 4748);
     svr->onConnRead([](const TcpConnPtr con){
         info("%s",con->getInput().data());
         con->send(con->getInput());
         con->sendOutput();
     });
     svr->onConnState([&](const TcpConnPtr& con) { //200ms后关闭连接
        if (con->getState() == TcpConn::Connected)
            base.runAfter(5, [con](){ info("close con after 200ms"); con->close(); });
    });
     base.loop();
     return 0;

    base.loop();
}
