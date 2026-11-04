#pragma once
#include "InetAddr.hpp"
#include "Logger.hpp"
#include <memory>
#include <string>
#include <iostream>

namespace SOCKETMOULD
{
    using namespace LoggerMould;
    static const int gbacklog = 15;
    enum
    {
        OK,
        SOCKET_ERR,
        BIND_ERR,
        LISTEN_ERR,
    };

    class Socket
    {
    public:
        Socket() {}
        ~Socket() {}

    protected:
        virtual void CreateSocketOrDie() = 0;
        virtual void BindSocketOrDie(uint16_t port) = 0;
        virtual void ListenSocketOrDie() = 0;

    public:
        virtual std::shared_ptr<Socket> Accepter(InetAddr &addr) = 0;
        virtual int Sockfd() = 0;
        virtual int Recv(std::string *out) = 0; // 这是写到网络里面去--具体什么内容要弄出来 给下面的使用
        virtual int Send(std::string &in) = 0;  // 从把网络文件里的东西发到外面去
        virtual void Close() = 0;
        virtual bool Connect(InetAddr &addr) = 0;

    public:
        void BuildTcpSocketMethod(uint16_t port) // 模版方法
        {
            CreateSocketOrDie();
            BindSocketOrDie(port);
            ListenSocketOrDie();
        }

        void BuildTcpClientSockMethod()
        {
            CreateSocketOrDie();
        }
    };

    class TcpSocket : public Socket
    {

    public:
        TcpSocket() : _sockfd(0)
        {
        }
        TcpSocket(int sockfd) : _sockfd(sockfd)
        {
        }

        ~TcpSocket()
        {
        }

    protected:
        void CreateSocketOrDie()
        {
            // 1.创建网络文件 返回文件描述符
            _sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (_sockfd < 0)
            {
                LOG(LogLevel::FATAL) << "套接字创建失败!!!";
                exit(SOCKET_ERR);
            }
            LOG(LogLevel::INFO) << "套接字创建成功: " << _sockfd;
        }
        void BindSocketOrDie(uint16_t port)
        {
            InetAddr sa(port);
            // 3.把结构体和网络文件绑定起来
            int n = bind(_sockfd, sa.NetAddress(), sa.Len());
            if (n < 0)
            {
                LOG(LogLevel::ERROR) << "套接字关联失败...";
                exit(BIND_ERR);
            }
            LOG(LogLevel::INFO) << "套接字关联成功: " << port;
        }
        void ListenSocketOrDie()
        {
            // 4.tcp是面向连接的 服务端需要处于一直监听的状态进行等待客户端连接
            int n = listen(_sockfd, gbacklog); // 告诉内核“我在这个端口等连接”
            if (n < 0)
            {
                LOG(LogLevel::FATAL) << "listen fail....!";
                exit(LISTEN_ERR);
            }
            LOG(LogLevel::INFO) << "listen success...!";
        }

    public:
        std::shared_ptr<Socket> Accepter(InetAddr &clientaddr)
        {
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            // 获取连接
            int sockfd = accept(_sockfd,CONV(&addr),&len);
            // 从已完成连接的队列里取出一个连接，返回一个新的 socket
            // accept() 返回的是新 socket，和监听 socket 不是同一个 监听 socket 只负责收连接，新 socket 才负责这个连接上的收发
            // 把地址强转成函数参数需要的形式即可
            //  收到信息之后进行回显发送回去
            if (sockfd < 0)
            {
                LOG(LogLevel::WARNING) << "accept erro!";
                return nullptr;
            }
            LOG(LogLevel::DEBUG) << "accept success, sockfd:" << sockfd;
            clientaddr = addr;
            return std::make_shared<TcpSocket>(sockfd);
        }

        int Sockfd() override
        {
            return _sockfd;
        }

        int Recv(std::string *out) override
        {
            char inbuff[10240] = {0};
            ssize_t n = recv(_sockfd, inbuff, sizeof(inbuff) - 1, 0); 
            if (n > 0)
            {
                inbuff[n] = 0;
                *out += inbuff;
            }
            return n;
        }

        int Send(std::string &in) override
        {
            return send(_sockfd, in.c_str(), in.size(), 0);
        }

        void Close() override
        {
            if (_sockfd >= 0)
            {
                close(_sockfd);
                _sockfd = -1;
            }
        }

        bool Connect(InetAddr &addr) override
        {
            int n = connect(_sockfd, addr.NetAddress(), addr.Len());
            if (n < 0)
                return false;
            return true;
        }

    public:
        int _sockfd;
    };

}