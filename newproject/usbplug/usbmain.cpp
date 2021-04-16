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
    atomic<int> clientNum(0);
    MultiBase base(10);
    TcpServerPtr tcpserver = TcpServer::startServer(base.allocBase(), "192.168.31.162", 4746);
    tcpserver->onConnState([](const TcpConnPtr& con) {
        if (con->getState() == TcpConn::Connected)
            info("a tcp clinet connect!!!!!");
        });

    HttpServer hserver(base.allocBase());
    int r = hserver.bind("192.168.31.162",8081);
    exitif(r, "bind failed");
    hserver.onRequest("GET", "/", [&](const HttpConnPtr& con)
    {
        con.getResponse().setStatus(200, "OK");
        con.sendFile("../web/index.html");
    });
    hserver.onRequest("GET", "/getNum", [&](const HttpConnPtr& con)
    {
            info("server GetNum:");
            HttpResponse& resp = con.getResponse();
            resp.getStatus() = 200;
            resp.getStatusWord() = "OK";
            resp.getBodys() = util::format("<html>\r\n<body>\r\n<h2> %d </ h2>\r\n<p> welcome to use this web </p>\r\n</body> \r\n</html>\r\n", (const int)clientNum);

            con.sendResponse();
    });
    hserver.onRequest("GET", "/123.jpg", [&](const HttpConnPtr& con)
    {
            info("server GetJpg:");
            con.sendFile("../web/123.jpg");
    });
    hserver.onRequest("POST", "/form", [&](const HttpConnPtr& con)
    {/*
        cout << "request:===============begin==================" << endl;;
        for(auto& t : con.getRequest().getHeaders())
        {
            cout << t.first << ": " << t.second << endl;
        }
        cout << "args::::::" << endl;
        for(auto& t : con.getRequest().getArgs())
        {
            cout << t.first << "------ " << t.second << endl;
        }
        auto username = con.getRequest().getArgs().find("username");
        auto password = con.getRequest().getArgs().find("password");
        if(username != con.getRequest().getArgs().end() && password != con.getRequest().getArgs().end())
        {
            if((username->second.compare("tony") == 0) && (password->second.compare("cwb1994228") == 0))
            {
                HttpResponse& resp = con.getResponse();
                resp.getStatus() = 200;
                resp.getStatusWord() = "OK";
                resp.getBodys() = util::format("<html>\r\n<body>\r\n<h2> %d </ h2>\r\n<p> welcome to use this web </p>\r\n</body> \r\n</html>\r\n", (const int)clientNum);

                con.sendResponse();
            }
        }
        //cout << con.getRequest().getBody().toString() << endl;
        cout << "request:=============end====================" << endl;*/
    });
    hserver.onConnState([&](const TcpConnPtr& con){
        if(con->getState() == TcpConn::Connected)
        {
            clientNum++;
            info("a clinet connect to server!, the total connectClinet is %d", (const int)clientNum);
            //con->addIdleCB(20, [](const TcpConnPtr& c){c->close();});
            //info("total client is %d",hserver.getBase()->);
        }
        else if (con->getState() == TcpConn::Closed)
        {
            clientNum --;
            info("a clinet DIS connect to server!, the total connectClinet is %d", (const int)clientNum);
        }
    });
    base.loop();
}
