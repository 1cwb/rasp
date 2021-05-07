#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <regex>
#include <chrono>
#include "include/json.h"
#include "config.h"
#include "common.h"
#include "util.h"
#include "mlog.h"
#include "rasp_impl.h"
#include "event_base.h"
#include "conn.h"
#include "status.h"
#include "file.h"
#include "http.h"
#include "protocol.h"

using namespace std;
using namespace rasp;

int main(int argc, char** argv)
{
    TimerId timeid;
    EventBase base;
    TcpConnPtr con = TcpConn::createConnection(base.allocBase(), BIG_IOT_WEB, PORT);
    BigIot bigiot(con);
    bigiot.onState([](BigIot& iot){
        if(iot.getTcpConnPtr()->getState() == TcpConn::Connected)
        {
            iot.sendCheckin("8350","aca4b50ba");
        }
    });
    
    bigiot.onRead([&](BigIot& iot){
        switch(iot.getMethod())
        {
            case E_METHOD::E_M_CHECKINOK:
                cout<< "device name" << iot.getDeviceName()<<endl;
                cout << "checkedn in " <<iot.deviceCheckin() <<endl;
                iot.sendCheckOnLine(vector<string> {"U5509"});
            break;
            case E_METHOD::E_M_LOGIN:
                cout << "USER " << iot.getClientName() << " " <<iot.getClientCId() << " login" <<endl;
                {
                    shared_ptr<BigIot> temp(new BigIot());
                    *temp.get() = iot;
                    timeid = base.runAfter(1000,[=](){
                    temp->sendSay(iot.getClientCId(),"------------");
                },1000);
                }

            break;
            case E_METHOD::E_M_LOGOUT:
                cout << "USER " << iot.getClientName() << " " <<iot.getClientCId() << " logout" <<endl;
                base.cancel(timeid);
            break;
            case E_METHOD::E_M_SAY:
                cout << "USER " << iot.getClientName() << " " <<iot.getClientCId() << "say: "<<iot.getSayContent() <<endl;
                iot.sendSay(iot.getClientCId(),"sha bi wan yi");
                iot.sendSay(iot.getClientCId(),"sha bi wan yi");
                iot.sendSay(iot.getClientCId(),"sha bi wan yi");
            break;
            case E_METHOD::E_M_TIME:
                cout << iot.getServerTime() <<endl;
            break;
            case E_METHOD::E_M_ISOL:
                if(iot.clientLogin())
                {
                    shared_ptr<BigIot> temp(new BigIot());
                        *temp.get() = iot;
                        timeid = base.runAfter(1000,[=](){
                        temp->sendSay(iot.getClientCId(),"------------");
                    },1000);
                }
                break;
            default:

            break;
        }
    });
    base.runAfter(1000,[&](){
        bigiot.sendHeartBeatPackage();
    },45000);

    base.runAfter(6000, [&](){
        bigiot.sendRealTimeData(map<string, string>{{"7274", "22"}});
    },6000);

    base.runAfter(10000, [&](){
        bigiot.sendCheckMyself();
    },10000);
   
    base.loop();
}
