#include <iostream>
#include "config.h"
#include "common.h"
#include "util.h"
#include <chrono>
#include "mlog.h"
#include "rasp_impl.h"
#include "event_base.h"
#include "conn.h"
#include "status.h"
#include "file.h"
using namespace std;
using namespace rasp;

int main(int argc, char** argv)
{
     EventBase base;
     TcpServerPtr svr = TcpServer::startServer(&base, "192.168.31.28", 4748);
     svr->onConnRead([](const TcpConnPtr con){
         info("%s",con->getInput().data());
         info("input buffer size %d",con->getInput().size());
         //if(con->getInput().size() == 0)
         //con->getInput().clear();
         //con->send(con->getInput());
         //con->sendOutput();
     });
     svr->onConnState([&](const TcpConnPtr& con) { //200ms后关闭连接
        if (con->getState() == TcpConn::State::Connected)
        {
            info("%s connected",con->peer_.toString().c_str());
        }
    });
     base.loop();
     return 0;

    base.loop();
}
