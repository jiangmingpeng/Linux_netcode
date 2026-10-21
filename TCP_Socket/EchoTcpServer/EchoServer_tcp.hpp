#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <functional>
#include <pthread.h>
#include "Logger.hpp"
#include "InetAddr.hpp"
#include "ThreadPool.hpp"

const static int default_fd = -1;
const static int default_port = 8888;
const static int gbacklog = 5; // 服务最大监听数量

using task_t = std::function<void()>;

using namespace ThreadPoolMould;
using namespace LoggerMould;

class TcpServer // 即创建一个UDP网络服务
{
    enum // 既然不指定类域 在这个文件里用来全局使用 就没必要进行枚举空间命名了
    {
        SUCCESS = 0,
        USAGE_ERR,
        SOCKET_ERR,
        BIND_ERR,
        LISTEN_ERR,
    };

public:
    TcpServer(uint16_t port = default_port)
        : _listensockfd(default_fd), _port(port)
    {
    }
    ~TcpServer()
    {
        close(_listensockfd);
    }

    void Init()
    {
        // 1.创建网络文件 返回文件描述符
        _listensockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (_listensockfd < 0)
        {
            LOG(LogLevel::FATAL) << "套接字创建失败!!!";
            exit(SOCKET_ERR);
        }
        LOG(LogLevel::INFO) << "套接字创建成功: " << _listensockfd;

        // 2.创建结构体用来 传端口号
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa)); // 把结构体置零 有个旧函数bzero也发挥同样的作用 但是可移植性差
        sa.sin_family = AF_INET;
        sa.sin_port = htons(_port);
        // sa.sin_addr.s_addr = inet_addr(_ip.c_str()); // 1. 字符串ip->4字节IP 2. hton
        sa.sin_addr.s_addr = INADDR_ANY; // 最佳实践:任意IP地址bind

        // 3.把结构体和网络文件绑定起来
        int n = bind(_listensockfd, (struct sockaddr *)&sa, sizeof(sa));
        if (n < 0)
        {
            LOG(LogLevel::ERROR) << "套接字关联失败...";
            exit(BIND_ERR);
        }
        LOG(LogLevel::INFO) << "套接字关联成功: " << _port;
        // 4.tcp是面向连接的 服务端需要处于一直监听的状态进行等待客户端连接
        n = listen(_listensockfd, gbacklog); // 告诉内核“我在这个端口等连接”
        if (n < 0)
        {
            LOG(LogLevel::FATAL) << "listen fail....!";
            exit(LISTEN_ERR);
        }
        LOG(LogLevel::INFO) << "listen success...!";
    }

    class ThreadData
    {
    public:
        ThreadData(TcpServer *ts, int sockfd, InetAddr &addr) : _this(ts), _sockfd(sockfd), _addr(addr)
        {
        }
        ~ThreadData()
        {
            close(_sockfd);
        }

        TcpServer *_this;
        int _sockfd;
        InetAddr _addr;
    };

    static void *thread_routine(void *arg)
    {
        ThreadData *td = static_cast<ThreadData *>(arg);
        pthread_detach(pthread_self());
        td->_this->ServerIO(td->_sockfd, td->_addr); // 每个线程进来启动完服务然后回收
        delete td;
        return nullptr;
    }

    void Start()
    {
        while (true)
        {
            // 这是为接受数据做的准备 那么这个结构体肯定是个输入输出型的 输出肯定是从客户端拿来的
            struct sockaddr_in client_peer;
            memset(&client_peer, 0, sizeof(client_peer));
            socklen_t len = sizeof(client_peer);
            // 获取连接
            int sockfd = accept(_listensockfd, (struct sockaddr *)&client_peer, &len);
            // 从已完成连接的队列里取出一个连接，返回一个新的 socket
            // accept() 返回的是新 socket，和监听 socket 不是同一个 监听 socket 只负责收连接，新 socket 才负责这个连接上的收发
            // 把地址强转成函数参数需要的形式即可
            //  收到信息之后进行回显发送回去
            if (sockfd < 0)
            {
                LOG(LogLevel::WARNING) << "accept erro!";
                continue; // 没有连接机器就一直尝试重新接收连接 对新连接进行收发
            }
            LOG(LogLevel::DEBUG) << "accept success, sockfd:" << sockfd;

            InetAddr clientaddress(client_peer);
            ThreadPool<task_t>::Instance()->Push
            (
                [this, sockfd, clientaddress]() -> void
                {
                    this->ServerIO(sockfd, clientaddress);
                }
            );
        }
    }

    void ServerIO(int sockfd, InetAddr address) // 放入线程池对新接收连接的sockfd进行收发信息
    {
        //第二个参数用来进行客户端辨认的在业务逻辑上不起作用
        char inbuff[1024]; // 用来存储接受到的字符串
        while (true)
        {
            // 从网络文件里读到信息之后进行回显发送回去
            ssize_t n = read(sockfd, inbuff, sizeof(inbuff)-1);
            if (n > 0)
            {

                inbuff[n] = '\0'; // 把读到的信息用\0断开
                LOG(LogLevel::INFO) << address.ToString() << " say:" << inbuff;
                // 回显对方信息(写)
                std::string echo_string = "Server echo: ";
                echo_string += inbuff;
                ssize_t size = write(sockfd, echo_string.c_str(), echo_string.size());
            }
            else if (n == 0) // 网络文件里没有信息可读
            {
                LOG(LogLevel::INFO) << "client quit, address:"<< address.ToString();
                break;
            }
            else
            {
                LOG(LogLevel::WARNING) << "数据接受失败 请重新发送...";
                break;
            }
        }
        close(sockfd);
    }

private:
    int _listensockfd;
    uint16_t _port;
    // std::string _ip;
};
