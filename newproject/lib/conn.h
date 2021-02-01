#pragma once
#include "event_base.h"

namespace rasp
{
    struct TcpConn: public std::enable_shared_from_this<TcpConn>, private noncopyable
    {
        enum State
        {
            Invalid = 1,
            HandShaking,
            Connected,
            Closed,
            Failed
        };

        TcpConn();
        virtual ~TcpConn();
        template<typename C = TcpConn>
        static TcpConnPtr createConnection(EventBase* base, const std::string& host, short port, int timeout = 0, const std::string& localip = "")
        {
            TcpConnPtr con(new C);
            con->connect(base, host, port, timeout, localip);
            return con;
        }
        template<typename C = TcpConn>
        static TcpConnPtr createConnection(EventBase* base, int fd, Ip4Addr local, Ip4Addr peer)
        {
            TcpConnPtr con(new C);
            con->attach(base, fd, local, peer);
            return con;
        }
        bool isClient() {return destPort_ > 0;}
        template<typename T> T& context() {return ctx_.context<T>();}
        EventBase* getBase() {return base_;}
        State getState() {return state_;}

        //TcpConn in/out buffer
        Buffer& getInput() {return input_;}
        Buffer& getOutput() {return output_;}

        Channel* getChannel() {return channel_;}
        bool writable() {return channel_ ? channel_->writeEnabled() : false;}

        //send data
        void sendOutput() {send(output_);}
        void send(Buffer& buff);
        void send(const char* buff, ssize_t len);
        void send(const std::string& s){send(s.data(), s.size());};
        void send(const char* s){send(s, strlen(s));}

        //call back when get data
        void onRead(const TcpCallBack& cb) {assert(!readcb_); readcb_ = cb;}
        // tcp buffer can be write call back
        void onWritable(const TcpCallBack& cb) {writablecb_ = cb;}
        //tcp state change call
        void onState(const TcpCallBack& cb) {statecb_ = cb;}
        //tcp free call
        void addIdleCB(int idle, const TcpCallBack& cb);

        //msg callback, just use one of onMsg and onRead
        void onMsg(CodecBase* codec, const MsgCallBack& cb);
        //send msg
        void sendMsg(Slice msg);

        void close();
        //set reconnect time, -1:never reconnect, 0:reconnect now, other value: wait ms on connect
        void setReconnectInterval(int milli){reconnectInterval_ = milli;}

        //! Warning: becareful
        void closeNow() {if(channel_){channel_->close();}}

        //peer addr str
        std::string str() {return peer_.toString();}
    public:
        EventBase* base_;
        Channel* channel_;
        Buffer input_, output_;
        Ip4Addr local_, peer_;
        State state_;
        TcpCallBack readcb_, writablecb_, statecb_;
        std::list<IdleId> idleIds_;
        TimerId timeOutId_;
        AutoContext ctx_, internalCtx_;//auto delete
        std::string destHost_, localIp_;
        int destPort_, connectTimeout_, reconnectInterval_;
        int64_t connectedTime_;
        std::unique_ptr<CodecBase> codec_;
        void handleRead(const TcpConnPtr& con);
        void handleWrite(const TcpConnPtr& con);
        ssize_t isend(const char* buf, ssize_t len);
        void cleanup(const TcpConnPtr& con);
        void connect(EventBase* base, const std::string& host, short port, int timeout, const std::string& localip);
        void reconnect();
        void attach(EventBase* base, int fd, Ip4Addr local, Ip4Addr peer);
        virtual int readImp(int fd, void* buf, size_t bytes){return ::read(fd, buf, bytes);}
        virtual int writeImp(int fd, const void* buf, size_t bytes){return ::write(fd, buf, bytes);}
        virtual int handleHandshake(const TcpConnPtr& con);
    };
    struct TcpServer : private noncopyable
    {
        TcpServer(EventBase* bases);
        int bind(const std::string& host, short port, bool reusePort=false);
        static TcpServerPtr startServer(EventBase* bases, const std::string& host, short port, bool reusePort = false);
        ~TcpServer() {if(listen_channel_){delete listen_channel_;}}
        Ip4Addr getAddr() {return addr_;}
        EventBase* getBase() { return base_; }
        void onConnCreate(const std::function<TcpConnPtr()>& cb) {createcb_ = cb;}
        void onConnState(const TcpCallBack& cb){statecb_ = cb;}
        void onConnRead(const TcpCallBack& cb){readcb_ = cb; assert(!msgcb_);}
        //conflit with onConnRead callback
        void onConnMsg(CodecBase* codec, const MsgCallBack& cb) { codec_.reset(codec); msgcb_ = cb; assert(!readcb_); }
    private:
        EventBase* base_;
        EventBases* bases_;
        Ip4Addr addr_;
        Channel* listen_channel_;
        TcpCallBack statecb_, readcb_;
        MsgCallBack msgcb_;
        std::function<TcpConnPtr()> createcb_;
        std::unique_ptr<CodecBase> codec_;
        void handleAccept();
    };
}