#include "net.h"
#include "util.h"
#include "mlog.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <string>
#include <netinet/tcp.h>

using namespace std;
namespace rasp
{
    int net::setNonBlock(int fd, bool value)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if(flags < 0)
        {
            return errno;
        }
        if(value)
        {
            return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        }
        return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    }
    int net::setReuseAddr(int fd, bool value)
    {
        int flags = value;
        int len = sizeof(flags);
        return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flags, len);
    }
    int net::setReusePort(int fd, bool value)
    {
        int flags = value;
        int len = sizeof(flags);
        return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &flags, len);
    }
    int net::setNodelay(int fd, bool value)
    {
        int flags = value;
        int len = sizeof(flags);
        return setsockopt(fd, SOL_SOCKET, TCP_NODELAY, &flags, len);
    }
    Ip4Addr::Ip4Addr(const std::string& host, short port)
    {
        memset(&addr_, 0, sizeof(addr_));
        addr_.sin_family = AF_INET;
        addr_.sin_port = htons(port);
        if(host.size())
        {
            addr_.sin_addr = port::getHostByName(host);
        }
        else
        {
            addr_.sin_addr.s_addr = INADDR_ANY;
        }
        if(addr_.sin_addr.s_addr == INADDR_NONE)
        {
            error("can not resolve %s to ip", host.c_str());
        }
    }
    std::string Ip4Addr::toString() const
    {
        uint32_t uip = addr_.sin_addr.s_addr;
        return util::format("%d.%d.%d.%d:%d",
            (uip >> 0)&0xff,
            (uip >> 8)&0xff,
            (uip >> 16)&0xff,
            (uip >> 24)&0xff,
            ntohs(addr_.sin_port));
    }
    std::string Ip4Addr::ip() const
    {
        uint32_t uip = addr_.sin_addr.s_addr;
        return util::format("%d.%d.%d.%d",
            (uip >> 0)&0xff,
            (uip >> 8)&0xff,
            (uip >> 16)&0xff,
            (uip >> 24)&0xff);
    }
    short Ip4Addr::port() const
    {
        return ntohs(addr_.sin_port);
    }
    unsigned int Ip4Addr::ipInt() const
    {
        return ntohl(addr_.sin_addr.s_addr);
    }
    bool Ip4Addr::isIpValid() const
    {
        return addr_.sin_addr.s_addr != INADDR_NONE;
    }

    char* Buffer::makeRoom(size_t len)
    {
        if(e_ + len <= cap_)
        {

        }
        else if(size() + len < cap_ / 2)
        {
            moveHead();
        }
        else
        {
            expand(len);
        }
        return end();
    }
    Buffer& Buffer::absorb(Buffer& buf)
    {
        if(&buf != this)
        {
            if(size() == 0)
            {
                int len = sizeof(Buffer);
                char b[len];
                memcpy(b, this, len);
                memcpy(this, &buf, len);
                memcpy(reinterpret_cast<void*>(&buf), b, len);
                std::swap(exp_, buf.exp_);
            }
            else
            {
                append(buf.begin(), buf.size());
                buf.clear();
            }
        }
        return *this;
    }
    void Buffer::expand(size_t len)
    {
        size_t ncap = std::max(exp_, std::max(2*cap_, size() + len));
        char* p = new char[ncap];
        std::copy(begin(), end(), p);
        e_ -= b_;
        b_ = 0;
        delete[] buf_;
        buf_ = p;
        cap_ = ncap;
    }
    void Buffer::copyFrom(const Buffer& b)
    {
        memcpy(this, &b, sizeof(b));
        if(b.buf_)
        {
            buf_ = new char[cap_];
            memcpy(data(), b.begin(), b.size());
        }
    }
}