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
#include "http.h"
using namespace std;
using namespace rasp;

int main(int argc, char** argv)
{
     EventBase base;
     HttpServer hserver(&base);
     int r = hserver.bind("192.168.31.28",8081);
     exitif(r, "bind failed");
     hserver.onGet("/tonytest", [](const HttpConnPtr& con)
     {
         //string v = con.getRequest().getVersion();
         //HttpResponse resp;
         //resp.getBody1() = "hello worldxxx";
         con.getResponse().getBody1() = "hello world";
         con.sendResponse(con.getResponse());
         //con.sendFile("../web/tonytest.html");
     });
    hserver.onConnState([](const TcpConnPtr& con){
        if(con->getState() == TcpConn::Connected)
        {
            info("a clinet connect to server!");
        }
    });
     /*TcpServerPtr svr = TcpServer::startServer(&base, "192.168.31.28", 4748);
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
     return 0;*/

    base.loop();
}
