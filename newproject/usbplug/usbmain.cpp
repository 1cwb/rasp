#include <iostream>
#include <regex>
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
#include "json.cpp"

using json = nlohmann::json;
using namespace std;
using namespace rasp;

int main(int argc, char** argv)
{
    atomic<int> clientNum(0);
    vector<string> webfile;
    MultiBase base(10);
    //EventBase base;
    TcpConnPtr master = TcpConn::createConnection(base.allocBase(), "www.bigiot.net", 8181);
    json mj, login;
    mj["M"] = "b";
    login["M"] = "checkin";
    login["ID"] = "8350";
    login["K"] = "aca4b50ba";
    master->onState([&login](const TcpConnPtr& con){
        if(con->getState() == TcpConn::Connected)
        {
            cout << "login.dump" <<login.dump(0) <<endl;
            con->send((login.dump(0)+"\n").data(), login.dump(0).size() + 1);
        }
    });
    HttpServer hserver(base.allocBase());
    int r = hserver.bind("192.168.31.162",8081);
    exitif(r, "bind failed");
    hserver.onRequest("GET", "/", [&](const HttpConnPtr& con)
    {
        con.getResponse().setStatus(200, "OK");
        con.sendFile("../web/login.html");
    });
    hserver.onRequest("POST", "/home.html", [&](const HttpConnPtr& con)
    {

        con.getResponse().setStatus(200, "OK");
        con.sendFile("../web/login.html");
    });
    Status stat = File::getChildren("../web/login", &webfile);
    if(!stat.ok())
    {
        error("Can not open directory ../web/login"); 
    }
    else
    {
        for(auto& fileName : webfile)
        {
            cout << "filename is " << fileName <<endl;
            hserver.onRequest("GET", "/login/"+fileName, [&](const HttpConnPtr& con)
            {
                con.getResponse().setStatus(200, "OK");
                cout << "server send ========================== "<<"../web/login/"+fileName<<endl;
                con.sendFile("../web/login/"+fileName);
            });
        }  
    }
    
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
