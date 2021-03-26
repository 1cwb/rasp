#include "http.h"
#include "file.h"
#include "mlog.h"
#include "status.h"

using namespace std;

namespace rasp
{
    void HttpMsg::clear()
    {
        headers.clear();
        body.clear();
        body2.clear();
        complete_ = 0;
        contentLen_ = 0;
        scanned_ = 0;
        version = "HTTP/1.1";
    }
    std::string HttpMsg::getValueFromMap_(std::map<std::string, std::string> &m, const std::string &n)
    {
        auto p = m.find(n);
        return p == m.end() ? "" : p->second;
    }
    HttpMsg::Result HttpMsg::tryDecode_(Slice buf, bool copyBody, Slice* line1)
    {
        if(complete_)
        {
            return Complete;
        }
        if(!contentLen_) //contentLen = 0
        {
            const char* p = buf.begin();
            Slice req;
            while(buf.size() >= scanned_ + 4) //\r\n\r\n this is head end
            {
                if(p[scanned_] == '\r' && memcmp(p + scanned_, "\r\n\r\n", 4) == 0)
                {
                    req = Slice(p, p + scanned_); //get head and break
                    break;
                }
                scanned_ ++;
            }
            if(req.empty())
            {
                return NotCompelete;
            }

            *line1 = req.eatLine();//get first line
            while (req.size())
            {
                req.eat(2);//skip \r\n
                Slice ln = req.eatLine(); //get line
                Slice k = ln.eatWord();   //get word
                ln.trimSpace();           //skip space
                if(k.size() && ln.size() && k.back() == ':')
                {
                    for(size_t i = 0; i < k.size(); i++)
                    {
                        ((char*)k.data())[i] = tolower(k[i]);
                    }
                    headers[k.sub(0, -1)] = ln; //no ":" get a map data like host -> www.xxx.com
                }
                else if(k.empty() && ln.empty() && req.empty())
                {
                    break;
                }
                else
                {
                    error("bad http line: %.*s %.*s", (int) k.size(), k.data(), (int)ln.size(), ln.data());
                    return Error;
                }
            }
            scanned_ += 4;
            contentLen_ = atoi(getHeader("content-length").c_str());
            if(buf.size() < contentLen_ + scanned_ && getHeader("Expect").size())
            {
                return Continue100;
            }
        }
        if(!complete_ && buf.size() >= contentLen_ + scanned_)
        {
            if(copyBody)
            {
                body.assign(buf.data() + scanned_, contentLen_);
            }
            else
            {
                body2 = Slice(buf.data() + scanned_, contentLen_);
            }
            complete_ = true;
            scanned_ += contentLen_;
        }
        return complete_ ? Complete : NotCompelete;
    }

    //HttpRequest
    int HttpRequest::encode(Buffer& buf) //encode for request for client
    {
        size_t osz = buf.size();//get original size of buf
        char conlen[1024];
        char reqln[4096];
        memset(conlen, 0, sizeof(conlen));
        memset(reqln, 0, sizeof(reqln));
        snprintf(reqln, sizeof(reqln), "%s %s %s\r\n", method.c_str(), query_uri.c_str(), version.c_str()); //first line :GET / HTTP1.1
        buf.append(reqln); //add first line 
        for(auto &hd : headers) //get headrs: host:xxxx.com\r\n ...
        {
            buf.append(hd.first).append(": ").append(hd.second).append("\r\n");
        }
        buf.append("Connection: Keep-Alive\r\n");
        snprintf(conlen, sizeof(conlen), "Content-Length: %lu\r\n", getBody().size());
        buf.append(conlen);
        buf.append("\r\n").append(getBody());
        return buf.size() - osz;
    }
    HttpMsg::Result HttpRequest::tryDecode(Slice buf, bool copyBody)//for server
    {
        size_t i = 0;
        Slice ln1;
        Result r = tryDecode_(buf, copyBody, &ln1);//get head and first line
        if(ln1.size())
        {
            method = ln1.eatWord();//get method: GET/POST/XXX
            query_uri = ln1.eatWord(); //get query_uri: /
            version = ln1.eatWord(); //get version: HTTP1.1
            if(query_uri.size() == 0 || query_uri[0] != '/')
            {
                error("query uri '%.*s' should begin with /", (int) query_uri.size(), query_uri.data());
                return Error;
            }
            for(i = 0; i < query_uri.size(); i++)
            {
                if(query_uri[i] == '?')// find "?" in query_uri::::Ex: http://www.it315.org/counter.jsp?name=zhangsan&password=123
                {
                    uri = Slice(query_uri.data(), i); //get first data to data+i for uri
                    Slice qs = Slice(query_uri.data() + i + 1, query_uri.size() - i - 1);//skip ? and get args
                    size_t c, kb, ke, vb, ve;
                    c = kb = ke = vb = ve = 0;
                    while (c < qs.size())
                    {
                        while (c < qs.size() && qs[c] != '=' && qs[c] != '&')
                        {
                            c ++;
                        }
                        ke = c;
                        if(c < qs.size() && qs[c] == '=')
                        {
                            c ++;
                        }
                        vb = c;
                        while (c < qs.size() && qs[c] != '&')
                        {
                            c ++;
                        }
                        ve = c;
                        if(c < qs.size() && qs[c] == '&')
                        {
                            c ++;
                        }
                        if(kb != ke)
                        {
                            args[string(qs.data() + kb, qs.data() + ke)] = string(qs.data() + vb, qs.data() + ve);
                        }
                        ve = vb = ke = kb = c;
                    }
                    break;
                }
            }
            if(i == query_uri.size())
            {
                uri = query_uri;
            }
        }
        return r;
    }
    int HttpResponse::encode(Buffer& buf) //for server
    {
        size_t osz = buf.size();
        char conlen[1024];
        char statusln[1024];
        memset(conlen, 0, sizeof(conlen));
        memset(statusln, 0, sizeof(statusln));
        snprintf(statusln, sizeof(statusln), "%s %d %s\r\n", version.c_str(), status, statusWord.c_str());//HTTP/1.1 200 OK\r\n
        buf.append(statusln);
        for(auto &hd : headers) //append head
        {
            buf.append(hd.first).append(": ").append(hd.second).append("\r\n");
        }
        buf.append("Connection: Keep-Alive\r\n");
        snprintf(conlen, sizeof(conlen), "Content-Length: %lu\r\n", getBody().size());
        buf.append(conlen);
        buf.append("\r\n").append(getBody());
        return buf.size() - osz;
    }
    HttpMsg::Result HttpResponse::tryDecode(Slice buf, bool copyBody)//for client
    {
        Slice ln1;
        Result r = tryDecode_(buf, copyBody, &ln1);
        if(ln1.size())
        {
            version = ln1.eatWord();
            status = atoi(ln1.eatWord().data());
            statusWord = ln1.trimSpace();
        }
        return r;
    }

    //HttpConnPtr
    void HttpConnPtr::sendFile(const std::string& filename) const
    {
        string cont;
        Status st = File::getContent(filename, cont);
        HttpResponse& resp = getResponse();
        if(st.code() == ENOENT)
        {
            resp.setNotFound();
        }
        else if(st.code())
        {
            resp.setStatus(500, st.msg());
        }
        else //st.code == 0
        {
            resp.getBody2() = cont;
        }
        sendResponse();
    }
    void HttpConnPtr::clearData() const
    {
        if(tcp->isClient())
        {
            tcp->getInput().consume(getResponse().getByte()); //client use respocse buffer
            getResponse().clear();
        }
        else
        {
            tcp->getInput().consume(getRequest().getByte()); //server use request buffer
            getRequest().clear();
        }
    }
    //register callback
    // tcp->handleRead() -> onRead() -> http(con)->handleRead-> (param HttpCallBack function cb)
    void HttpConnPtr::onHttpMsg(const HttpCallBack& cb) const 
    {
        tcp->onRead([cb](const TcpConnPtr& con)
        {
            HttpConnPtr hcon(con); //if this is server, it need send data to diffrent connects
            hcon.handleRead(cb);
        });
    }

    void HttpConnPtr::handleRead(const HttpCallBack& cb) const
    {
        if(!tcp->isClient())//server
        {
            HttpRequest& req = getRequest();
            HttpMsg::Result r = req.tryDecode(tcp->getInput());
            if(r == HttpMsg::Error)
            {
                tcp->close();
                return;
            }
            if(r == HttpMsg::Continue100)
            {
                tcp->send("HTTP/1.1 100 Continue\r\n\r\n");
            }
            else if(r == HttpMsg::Complete)
            {
                info("http request: %s %s %s", req.getMethod().c_str(), req.getQureUri().c_str(), req.getVersion().c_str());
                trace("http request:\n%.*s", (int) tcp->input_.size(), tcp->input_.data());
                cout << "this ====" << this << endl;
                cb(*this);
            }
        }
        else
        {
            HttpResponse& resp = getResponse();
            HttpMsg::Result r = resp.tryDecode(tcp->getInput());
            if(r == HttpMsg::Error)
            {
                tcp->close();
                return;
            }
            if(r == HttpMsg::Complete)
            {
                info("http response: %d %s", resp.getStatus(), resp.getStatusWord().c_str());
                trace("http response:\n%.*s", (int) tcp->input_.size(), tcp->input_.data());
                cb(*this);
            }
        }
    }
    void HttpConnPtr::logOutput(const char* title) const
    {
        Buffer &o = tcp->getOutput();
        info("%s:\n%.*s", title, (int)o.size(), o.data());
    }

    //HttpServer
    HttpServer::HttpServer(EventBases* bases) : TcpServer(bases)
    {
        defcb_ = [](const HttpConnPtr& con) //if not found it will call defcb_
        {
            HttpResponse & resp = con.getResponse();
            resp.getStatus() = 404;
            resp.getStatusWord() = "Not Found";
            resp.getBody1() = "Not Found";
            con.sendResponse();
        };
        conncb_ = [] {return TcpConnPtr(new TcpConn);}; //default conncb_
        onConnCreate([this]() //this callback use in tcpserver
        {
            HttpConnPtr hcon(conncb_());
            hcon.onHttpMsg([this](const HttpConnPtr& hcon)
            {
                HttpRequest& req = hcon.getRequest();
                auto p = cbs_.find(req.getMethod());
                if(p != cbs_.end())
                {
                    auto p2 = p->second.find(req.getUri());
                    if(p2 != p->second.end())
                    {
                        p2->second(hcon);
                        return;
                    }
                }
                defcb_(hcon);
            });
            return hcon;
        });
    }
}