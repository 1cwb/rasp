#include <iostream>
#include <map>
#include <string>
#include <vector>
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
#include "protocol.h"

using json = nlohmann::json;
using namespace std;
using namespace rasp;

int main(int argc, char** argv)
{
    EventBase base;
    BigIot bigiot(base.allocBase());
    bigiot.connect();

    string id = "8350";

    bigiot.onState([](BigIot& iot){
        if(iot.getTcpConnPtr()->getState() == TcpConn::Connected)
        {
            iot.sendCheckin("8350","aca4b50ba");
        }
        
    });
    
    
    bigiot.onRead([](BigIot& iot){
        //cout << "metadata:"<<iot.getTcpConnPtr()->getInput().data() <<endl;
        //cout <<"================="<<endl;
        //iot.getTcpConnPtr()->getInput().clear();
        //string say = "{\"M\":\"say\",\"ID\":\"U5509\",\"C\":\"sa bi\",\"SIGN\":\"sa diao\"}\n";
        //iot.getTcpConnPtr()->send(say.data(), say.size());
        iot.sendSay("U5509", "fuck you every day", "fuck");
    });
    base.runAfter(45000,[&](){
        bigiot.getTcpConnPtr()->send("{\"M\":\"b\"}\n");
    },45000);

    base.runAfter(6000, [&](){
        bigiot.sendRealTimeData(map<string, string>{{"7274", "22"}});
    },6000);

    base.runAfter(10000, [&](){
        bigiot.sendCheckMyself();
    },10000);
    base.runAfter(10000, [&](){
        bigiot.sendWarning("1051","fuck you now!");
        bigiot.snedGetServerTime();
    },10000);
    base.loop();
}
