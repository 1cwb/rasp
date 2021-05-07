#include "protocol.h"

using namespace std;

namespace rasp
{
    void BigIot::sendHeartBeatPackage()
    {
        if(con_)
        {
            con_->send("{\"M\":\"b\"}\n");
        }
    }
    void BigIot::sendCheckin(const std::string& deviceId, const std::string& apiKey)
    {
        if(con_)
        {
            deviceId_ = deviceId;
            apikey_ = apiKey;
            string checkin = "{\"M\":\"checkin\",\"ID\":";
            checkin.append("\"");
            checkin.append(deviceId);
            checkin.append("\",");
            checkin.append("\"K\":");
            checkin.append("\"");
            checkin.append(apiKey);
            checkin.append("\"}\n");
            //cout << checkin <<endl;
            con_->send(checkin);
        }
    }
    void BigIot::sendRealTimeData(const std::map<std::string, std::string>& data)
    {
        if(con_)
        {
            string buff = "{\"M\":\"update\",\"ID\":";
            buff.append("\"");
            buff.append(deviceId_);
            buff.append("\",");
            buff.append("\"V\":");
            buff.append("{");
            for(auto& param : data)
            {

                buff.append("\"");
                buff.append(param.first.data());
                buff.append("\":");
                buff.append("\"");
                buff.append(param.second.data());
                buff.append("\"");
                if(param != *data.rbegin())
                {
                    buff.append(",");
                }
            }
            buff.append("}");
            buff.append("}\n");
            //cout << buff <<endl;
            con_->send(buff);
        }
    }
    void BigIot::sendSay(const std::string& id, const std::string& content, const std::string& label)
    {
        if(con_)
        {
            string buff = "{\"M\":\"say\",\"ID\":";
            buff.append("\"");
            buff.append(id);
            buff.append("\",");
            buff.append("\"C\":");
            buff.append("\"");
            buff.append(content);
            buff.append("\",");
            buff.append("\"SIGN\":");
            buff.append("\"");
            buff.append(label);
            buff.append("\"}\n");
           //cout << buff <<endl;
            con_->send(buff);
        }
    }
    void BigIot::sendCheckOnLine(const std::vector<string>& id)
    {
        if(con_)
        {
            string buff = "{\"M\":\"isOL\",\"ID\":";
            buff.append("[");
            for(auto& mid : id)
            {
                buff.append("\"");
                buff.append(mid);
                buff.append("\"");
                if(mid != *id.rbegin())
                {
                    buff.append(",");
                }
            }
            buff.append("]");
            buff.append("}\n");
            //cout << buff <<endl;
            con_->send(buff);
        }
    }
    void BigIot::sendCheckMyself()
    {
        if(con_)
        {
            string buff = "{\"M\":\"status\"}\n";
            //cout << buff <<endl;
            con_->send(buff);
        }
    }
    void BigIot::sendWarning(const std::string& id, const std::string& content)
    {
        if(con_)
        {
            string buff = "{\"M\":\"alert\",\"ID\":";
            buff.append("\"");
            buff.append(id);
            buff.append("\",");
            buff.append("\"C\":");
            buff.append("\"");
            buff.append(content);
            buff.append("\"}\n");
            //cout << buff <<endl;
            con_->send(buff);
        }
    }
    void BigIot::snedGetServerTime(const BigIot::TimeFormat& timeFormat)
    {
        if(con_)
        {
            string buff = "{\"M\":\"time\",\"F\":";
            buff.append("\"");
            switch (timeFormat)
            {
            case TimeFormat::Stamp:
                buff.append("stamp");
                break;
            case TimeFormat::Y_M_D:
                buff.append("Y-m-d");
                break;
            case TimeFormat::Y_M_D_H_I_S:
                buff.append("Y-m-d H:i:s");
                break;
            case TimeFormat::Y_POINT_M_POINT_D:
                buff.append("Y.m.d");
                break;
            default:
                buff.append("Y-m-d H:i:s");
                break;
            }
            buff.append("\"}\n");
            //cout << buff <<endl;
            con_->send(buff);
        }
    }
    std::string BigIot::getValueFromJson(const std::string& buff, const std::string& key)
    {
        string value;
        size_t posb = 0;
        size_t pose = 0;
        posb = buff.find(key);
        if(posb == buff.npos)
        {
            return " ";
        }
        posb += key.size();
        pose = buff.find(BIGIOT_END,posb);
        if(pose == buff.npos)
        {
            return " ";
        }
        return buff.substr(posb, pose - posb);
    }
    void BigIot::parseData(const Buffer& buff)
    {
        std::string data;
        std::string method;
        data.assign(buff.data());
        method = getValueFromJson(data, BIGIOT_METHOD);
        if(method == "checkinok")
        {
            deviceName_ = getValueFromJson(data, BIGIOT_DEVICENAME);
            bDeviceCheckin_ = true;
            //cout <<deviceName_<<endl;
            emthod = E_M_CHECKINOK;
        }
        else if(method == "ping")
        {
            emthod = E_M_PING;
            this->sendHeartBeatPackage();
        }
        else if(method == "beat")
        {
            emthod = E_M_BEAT;
            this->sendHeartBeatPackage();
        }
        else if(method == "login")
        {
            clientCId_ = getValueFromJson(data, BIGIOT_CLIENTID);
            clientName_ = getValueFromJson(data, BIGIOT_CLIENTENAME);
            bClientLogin_ = true;
            //cout << clientCId_ <<": "<<clientName_<< "true"<<endl;
            emthod = E_M_LOGIN;
        }
        else if(method == "logout")
        {
            clientCId_ = getValueFromJson(data, BIGIOT_CLIENTID);
            clientName_ = getValueFromJson(data, BIGIOT_CLIENTENAME);
            bClientLogin_ = false;
            //cout << clientCId_ <<": "<<clientName_<< "false"<<endl;
            emthod = E_M_LOGOUT;
        }
        else if(method == "say")
        {
            clientCId_ = getValueFromJson(data, BIGIOT_CLIENTID);
            clientName_ = getValueFromJson(data, BIGIOT_CLIENTENAME);
            saycontent = getValueFromJson(data, BIGIOT_CONTENT);
            saysign = getValueFromJson(data, BIGIOT_SIGN);
            //cout << "1212--"<<saycontent <<endl;
            emthod = E_M_SAY;
        }
        else if(method == "isOL")
        {
            emthod = E_M_ISOL;
        }
        else if(method == "connected")
        {
            emthod = E_M_CONNECTED;
        }
        else if(method == "checked")
        {
            bDeviceCheckin_ = true;
            emthod = E_M_CHECKED;
        }
        else if(method == "time")
        {
            serverTime = getValueFromJson(data, BIGIOT_TIME);
            emthod = E_M_TIME;
            //cout<<"tonyxx" <<serverTime <<endl;
        }
        else
        {
            emthod = E_M_OTHERS;
        }
    }
}