#pragma once
#include <string>
#include <functional>
#include <map>
#include "event_base.h"
#include "conn.h"
#include "mlog.h"

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
#define BIGIOT_END            "\","

namespace rasp
{
    class BigIot;
    typedef std::function<void(BigIot& iot)> IotCallBack;
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
        BigIot(EventBase* base):base_(base),bDeviceCheckin_(false), bClientLogin_(false), emthod(E_M_OTHERS){}
        ~BigIot(){}

        void connect(const std::string& host = BIG_IOT_WEB, const short port = PORT)
        {
            con_ = TcpConn::createConnection(base_, host, port);
        }
        void onRead(const IotCallBack& cb)
        {
            con_->onRead([cb,this](const TcpConnPtr& con){
                this->onRead_(cb);
            });
        }
        void onState(const IotCallBack& cb)
        {
            con_->onState([cb,this](const TcpConnPtr& con){
                this->onState_(cb);
            });
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

        TcpConnPtr& getTcpConnPtr() {return con_;}
        private:
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
        void parseData(const Buffer& buff);
        std::string getValueFromJson(const std::string& buff, const std::string& key);
        TcpConnPtr con_;
        EventBase* base_;
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
}