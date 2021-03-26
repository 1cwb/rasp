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
     int r = hserver.bind("192.168.31.162",8081);
     exitif(r, "bind failed");
     hserver.onGet("/tonytest", [](const HttpConnPtr& con)
     {
         //string v = con.getRequest().getVersion();
         //HttpResponse resp;
         //resp.getBody1() = "hello worldxxx";
         //con.getResponse().getBody1() = "sha bi,,,,,,,";
         
         //con.sendResponse(con.getResponse());
         //con.getResponse().getHeaders()["Content-type"] = "image/jpeg";
         con.sendFile("../web/test.html");
         cout << "con addr" << &con << endl;
     });
     hserver.onRequest("GET", "/123.jpg", [](const HttpConnPtr& con)
     {
         con.sendFile("../web/123.jpg");
         cout << "con addr" << &con << endl;
     });
    hserver.onConnState([&hserver](const TcpConnPtr& con){
        if(con->getState() == TcpConn::Connected)
        {
            info("a clinet connect to server!");
            //con->addIdleCB(20, [](const TcpConnPtr& c){c->close();});
            //info("total client is %d",hserver.getBase()->);
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
    //base.runAfter(10000, [&](){base.exit();});
    
    base.loop();
}
