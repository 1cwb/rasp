#pragma once
#include <string>
#include <functional>
#include <map>
#include "event_base.h"
#include "conn.h"
#include "mlog.h"
#include "include/json.h"

#define BIG_IOT_WEB "www.bigiot.net"
#define PORT 8181

#define BIGIOT_METHOD         "\"M\":\""
#define BIGIOT_DEVICENAME     "\"NAME\":\""
#define BIGIOT_DEVICEID       "\"ID\":\""
#define BIGIOT_CLIENTID       "\"ID\":\""
#define BIGIOT_CLIENTENAME    "\"NAME\":\""
#define BIGIOT_CONTENT        "\"C\":\""
#define BIGIOT_SIGN           "\"SIGN\":\""
#define BIGIOT_TIME           "\"T\":\""
#define BIGIOT_END            "\""

namespace rasp
{
    class BigIot;
    using IotCallBack = std::function<void(BigIot& iot)>;
    
    class BigIot
    {
        public:
        enum Method
        {
            E_M_PING,
            E_M_BEAT,
            E_M_CHECKINOK,
            E_M_LOGIN,
            E_M_LOGOUT,
            E_M_SAY,
            E_M_ISOL,
            E_M_CONNECTED,
            E_M_CHECKED,
            E_M_TIME,
            E_M_OTHERS
        };
        enum TimeFormat
        {
            Stamp,
            Y_M_D,
            Y_POINT_M_POINT_D,
            Y_M_D_H_I_S
        };
        operator TcpConnPtr() {return con_;}
        operator TcpConn*() {return con_.get();}
        BigIot(TcpConnPtr con):con_(con),bDeviceCheckin_(false), bClientLogin_(false), emthod(E_M_OTHERS){}
        ~BigIot(){}

        /*void connect(const std::string& host = BIG_IOT_WEB, const short port = PORT)
        {
            con_ = TcpConn::createConnection(base_, host, port);
        }*/
        void onRead(const IotCallBack& cb)
        {
            con_->onRead([cb](const TcpConnPtr& con){
                BigIot bigiot(con);
                bigiot.onRead_(cb);
            });
        }
        void onState(const IotCallBack& cb)
        {
            con_->onState([cb](const TcpConnPtr& con){
                BigIot bigiot(con);
                bigiot.onState_(cb);
            });
        }
        void onRead_(const IotCallBack& cb)
        {   
            parseData(con_->getInput());
            cb(*this);
            con_->getInput().clear();
        }
        void onState_(const IotCallBack& cb)
        {
            cb(*this);
        }
        void sendHeartBeatPackage();
        void sendCheckin(const std::string& deviceId, const std::string& apiKey);
        void sendRealTimeData(const std::map<std::string, std::string>& data);
        void sendSay(const std::string& id, const std::string& content, const std::string& label);
        void sendCheckOnLine(const std::vector<std::string>& id);
        void sendCheckMyself();
        void sendWarning(const std::string& id, const std::string& content);
        void snedGetServerTime(const TimeFormat& timeFormat = TimeFormat::Y_M_D_H_I_S);

        bool deviceCheckin()const{return bDeviceCheckin_;}
        bool clientLogin()const{return bClientLogin_;}

        std::string getDeviceName()const{return deviceName_;}
        std::string getDeviceId()const{return deviceId_;}
        std::string getapiKey()const{return apikey_;}

        std::string getClientName()const{return clientName_;}
        std::string getClientUId()const{return clinetUId_;}
        std::string getClientCId()const{return clientCId_;}
        std::string getServerTime()const{return serverTime;}
        std::string getSayContent()const{return saycontent;}

        TcpConnPtr& getTcpConnPtr() {return con_;}
        Method getMethod() {return emthod;}
        private:
        
        void parseData(const Buffer& buff);
        std::string getValueFromJson(const std::string& buff, const std::string& key);
        TcpConnPtr con_;
        //EventBase* base_;
        bool bDeviceCheckin_;
        
        std::string deviceName_;
        std::string deviceId_;
        std::string apikey_;

        bool bClientLogin_;
        std::string clinetUId_;
        std::string clientCId_;
        std::string clientName_;
        std::string saycontent;
        std::string saysign;
        std::string serverTime;
        Method emthod;
    };
    using E_METHOD = BigIot::Method;
}