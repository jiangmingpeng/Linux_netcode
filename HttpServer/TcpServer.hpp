#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include <memory>
#include <functional>
#include <pthread.h>
#include "Logger.hpp"
#include "InetAddr.hpp"
#include "Socket.hpp"

const static int default_fd = -1;
const static int default_port = 8888;
const static int gbacklog = 5; // 服务最大监听数量

using task_t = std::function<void()>;
using Handler_t = std::function<std::string(std::string &)>; // 用来往上进行回调--大概率使用lamda表达式

using namespace LoggerMould;
using namespace SOCKETMOULD;
class TcpServer //
{
public:
    TcpServer(Handler_t handler, uint16_t port = default_port)
        : _port(port), _listensock(std::make_unique<TcpSocket>()), _handler(handler)
    {
        _listensock->BuildTcpSocketMethod(_port);
        LOG(LogLevel::INFO) << "create listen socket success: " << _listensock->Sockfd();
    }
    ~TcpServer()
    {
    }

    void Loop()
    {
        signal(SIGCHLD, SIG_IGN);
        while (true)
        {
            InetAddr clientaddr;
            auto sockfd = _listensock->Accepter(clientaddr);
            if (!sockfd)
            {
                continue;
            }
            LOG(LogLevel::DEBUG) << "get a new link, socket address: " << clientaddr.ToString() << " sockfd: " << sockfd->Sockfd();

            if (fork() == 0)
            {
                service(sockfd, clientaddr);
                sockfd->Close();
                exit(0);
            }
        }
    }

private:
    void service(std::shared_ptr<Socket> sockfd, InetAddr address)
    {
        std::string inbuff, outbuff;
        outbuff.clear();
        int n = 0;
        while ((n = sockfd->Recv(&inbuff)) > 0)
        {
            if (inbuff.find("\r\n\r\n") != std::string::npos)
                break; // 报头收全了，可以解析了 空格那里刚好确实可以读到两个\r\n\r\n

            if (inbuff.size() > 16 * 1024) // 防止畸形请求把缓冲区撑爆
            {
                LOG(LogLevel::ERROR) << "request header too large, drop it";
                return;
            }
        }
        if (n <= 0)
        {
            LOG(LogLevel::WARNING) << "recv: client quit," << address.ToString();
            return;
        }

         LOG(LogLevel::DEBUG) << "recv " << inbuff.size() << " bytes:\n" << inbuff;

        if (_handler)
            outbuff += _handler(inbuff);

        // if (outbuff.empty())
        //     continue;

        // LOG(LogLevel::DEBUG) << "outbuff:\n"
        //                         << outbuff;
        LOG(LogLevel::DEBUG) << "reply " << outbuff.size() << " bytes";
        n = sockfd->Send(outbuff);
        if (n < 0)
        {
            LOG(LogLevel::WARNING) << "send: client quit," << address.ToString();
            return;
        }
        
    }

private:
    uint16_t _port;
    std::unique_ptr<Socket> _listensock;
    Handler_t _handler;
};
