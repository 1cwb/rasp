#pragma once

#include <map>
#include "conn.h"
#include "slice.h"

namespace rasp
{
    struct HttpMsg
    {
        enum Result
        {
            Error,
            Complete,
            NotCompelete,
            Continue100
        };

        HttpMsg() {HttpMsg::clear();}
        virtual ~HttpMsg() {}
        virtual int encode(Buffer& buf) = 0;
        virtual HttpMsg::Result tryDecode(Slice buf, bool copyBody = true) = 0;
        virtual void clear();

        std::string getHeader(const std::string& n){return getValueFromMap_(headers, n);}
        Slice getBody() {return body2.size() ? body2 : Slice(body);}
        int getByte() {return scanned_;}
    
        std::map<std::string, std::string> headers;
        std::string body;
        Slice body2;
        std::string version;
    protected:
        bool complete_;
        size_t contentLen_;
        size_t scanned_;
        Result tryDecode_(Slice buf, bool copyBody, Slice* line1);
        std::string getValueFromMap_(std::map<std::string, std::string> &m, const std::string &n);
    };

    struct HttpRequest : public HttpMsg
    {
        HttpRequest() {clear();}
        virtual ~HttpRequest() {}
        virtual int encode(Buffer& buf) override;
        virtual HttpMsg::Result tryDecode(Slice buf, bool copyBody = true) override;
        virtual void clear()
        {
            HttpMsg::clear();
            args.clear();
            method = "GET";
            query_uri = "";
            uri = "";
        }
        std::map<std::string, std::string> args;
        std::string method;
        std::string uri;
        std::string query_uri;
    };

    struct HttpResponse : public HttpMsg
    {
        HttpResponse() {clear();}
        virtual ~HttpResponse(){}
        virtual int encode(Buffer& buf) override;
        virtual HttpMsg::Result tryDecode(Slice buf, bool copyBody = true) override;
        virtual void clear()
        {
            HttpMsg::clear();
            status = 200;
            statusWord = "OK";
        }

        void setNotFound() {setStatus(404, "Not Found");}
        void setStatus(int st, const std::string &msg = "") 
        {
            status = st;
            statusWord = msg;
            body = msg;
        }
        std::string statusWord; 
        int status;
    };

    struct HttpConnPtr
    {
        typedef std::function<void (const HttpConnPtr& )> HttpCallBack;
        HttpConnPtr(const TcpConnPtr& con): tcp(con) {}
        operator TcpConnPtr() const {return tcp;}
        TcpConn* operator->() const {return tcp.get();}
        bool operator<(const HttpConnPtr& con) const {return tcp < con.tcp;}

        HttpRequest& getRequest() const {return tcp->internalCtx_.context<HttpContext>().req;}
        HttpResponse& getResponse() const {return tcp->internalCtx_.context<HttpContext>().resp;}

        void sendRequest() const {sendRequest(getRequest());}
        void sendResponse() const {sendResponse(getResponse());}
        void sendRequest(HttpRequest& req) const
        {
            req.encode(tcp->getOutput());
            logOutput("http req");
            clearData();
            tcp->sendOutput();
        }
        void sendResponse(HttpResponse& resp) const
        {
            resp.encode(tcp->getOutput());
            logOutput("http resp");
            clearData();
            tcp->sendOutput();
        }
        void sendFile(const std::string& filename) const;
        void clearData() const;
        void onHttpMsg(const HttpCallBack& cb) const;
        void handleRead(const HttpCallBack& cb) const;
        void logOutput(const char* title) const;
    protected:
        struct HttpContext
        {
            HttpRequest req;
            HttpResponse resp;
        };
    private:
        TcpConnPtr tcp;
    };

    typedef HttpConnPtr::HttpCallBack HttpCallBack;
}