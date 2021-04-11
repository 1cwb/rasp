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
    TcpConnPtr con = TcpConn::createConnection(&base, "m2.5y1rsxmzh.club",80);
    //TcpConnPtr con = TcpConn::createConnection(&base, "www.baidu.com", 80);
    HttpConnPtr http(con);
    con->onState([&http](const TcpConnPtr& con){
        if(con->getState() == TcpConn::Connected)
        {
            http.getRequest().getMethod() = "GET";
            http.getRequest().getQureUri() = "/pw";
            http.getRequest().getHeaders()["Host"] = "m2.5y1rsxmzh.club";

            http.getRequest().getHeaders()["Referrer Policy"] = "strict - origin - when - cross - origin";
            http.getRequest().getHeaders()["Accept"] = "text / html, application / xhtml + xml, application / xml; q = 0.9, image / avif, image / webp, image / apng, */*;q=0.8,application/signed-exchange;v=b3;q=0.9";
            http.getRequest().getHeaders()["Accept-Encoding"] = "gzip, deflate";
            //http.getRequest().getHeaders()["Referer"] = "http://m2.5y1rsxmzh.club/";
            
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
