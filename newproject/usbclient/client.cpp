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
    EventBase base;
    TcpConnPtr con = TcpConn::createConnection(&base, "192.168.31.162",8081);
    HttpConnPtr http(con);
    con->onState([&http](const TcpConnPtr& con){
        if(con->getState() == TcpConn::Connected)
        {
            http.getRequest().getMethod() = "GET";
            http.getRequest().getQureUri() = "/tonytest";
            //http.getRequest().getHeaders()["Host"] = "cdn3.61gp.xyz";
            http.sendRequest();
        }
    });
    http.onHttpMsg([](const HttpConnPtr& m){
        for(auto& t : m.getResponse().getHeaders())
        {
            cout << t.first << ": " << t.second << endl;
        }
        cout << m.getResponse().getBody().toString() << endl;
    });
    base.loop();
    return 0;
}
