#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <functional>
#include "InetAddr.hpp"
#include "Logger.hpp"



const static int default_fd = -1;
const static int default_port = 8888;

using namespace LoggerMould;

using handler_addr_t = std::function<void (const InetAddr &)>;
using handler_msg_t = std::function<void (int sokcfd, std::string msg)>;



class UdpServer // 即创建一个UDP网络服务
{
    enum // 既然不指定类域 在这个文件里用来全局使用 就没必要进行枚举空间命名了
    {
        SUCCESS = 0,
        USAGE_ERR,
        SOCKET_ERR,
        BIND_ERR,
    };

public:
    UdpServer(uint16_t port = default_port)
        : _sockfd(default_fd), _port(port)
    {
    }
    ~UdpServer()
    {
        close(_sockfd);
    }

    void Register(handler_addr_t handler_addr,handler_msg_t handler_msg)
    {
        _handler_addr = handler_addr;
        _handler_msg = handler_msg;
    }

    void Init()
    {
        // 1.创建网络文件 返回文件描述符
        _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (_sockfd < 0)
        {
            LOG(LogLevel::FATAL) << "套接字创建失败!!!";
            exit(SOCKET_ERR);
        }
        LOG(LogLevel::INFO) << "套接字创建成功: " << _sockfd;

        // 2.创建结构体用来 传端口号
        InetAddr local(_port);

        // 3.把结构体和网络文件绑定起来
        int n = bind(_sockfd, (struct sockaddr *)local.GetNetAddress(), local.Len());
        if (n < 0)
        {
            LOG(LogLevel::ERROR) << "套接字关联失败...";
            exit(BIND_ERR);
        }
        LOG(LogLevel::INFO) << "套接字关联成功: " << _port;
    }

    void Start()
    {
        char inbuff[1024];//用来存储接受到的字符串
        while(true)
        {
            //1. 这是为接受数据做的准备 那么这个结构体肯定是个输入输出型的 输出肯定是从客户端拿来的
            struct sockaddr_in client_peer;
            memset(&client_peer,0,sizeof(client_peer));
            //2. 从网络文件里接受文件(默认阻塞接受)
            socklen_t len = sizeof(client_peer);
            ssize_t n = recvfrom(_sockfd,inbuff,sizeof(inbuff)-1,0,(sockaddr*)&client_peer,&len);
            //3.收到信息之后进行回显发送回去
            if(n>0)
            {
                inbuff[n] = 0;
                //1.检测到新用户
                InetAddr clientaddress(client_peer);//每次检测到新用户都把他描述起来然后进行组织管理
                std::string tips = clientaddress.ToString();
                std::string message = tips + inbuff;
                LOG(LogLevel::DEBUG)<<message;
                _handler_addr(clientaddress);//这里放了两个回调函数
                //2.转发消息
                _handler_msg(_sockfd,message);
            }
            else
            {
                LOG(LogLevel::WARNING)<<"数据接受失败 请重新尝试...";
            }
        }
    }

private:
    int _sockfd;
    uint16_t _port;
    handler_addr_t _handler_addr = [](const InetAddr &) {};       // 默认空回调，Register 之前调用也安全
    handler_msg_t _handler_msg = [](int, std::string) {};          // 默认空回调
    // std::string _ip;
};
