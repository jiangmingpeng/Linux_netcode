#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include "Logger.hpp"
#include <functional>

const static int default_fd = -1;
const static int default_port = 8888;

using namespace LoggerMould;

using callback_t = std::function<std::string(std::string)>;

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
    // UdpServer(uint16_t port = default_port)
    UdpServer(callback_t cb,uint16_t port = default_port)
        : _sockfd(default_fd), _port(port),_cb(cb)
    {
    }
    ~UdpServer()
    {
        close(_sockfd);
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
        struct sockaddr_in sa;
        memset(&sa, 0, sizeof(sa)); // 把结构体置零 有个旧函数bzero也发挥同样的作用 但是可移植性差
        sa.sin_family = AF_INET;
        sa.sin_port = htons(_port);
        // sa.sin_addr.s_addr = inet_addr(_ip.c_str()); // 1. 字符串ip->4字节IP 2. hton
        sa.sin_addr.s_addr = INADDR_ANY; // 最佳实践:任意IP地址bind

        // 3.把结构体和网络文件绑定起来
        int n = bind(_sockfd, (struct sockaddr *)&sa, sizeof(sa));
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
                uint16_t client_port = ntohs(client_peer.sin_port);//要在主机上使用 就得转化字节序
                std::string client_ip = inet_ntoa(client_peer.sin_addr); // 4字节IP-> ntoh -> 字符串
                std::string client_address = "[" + client_ip + ":" + std::to_string(client_port) + "]#";

                inbuff[n] = '\0';//把读到的信息用\0断开---这里收到的是客户端传过来的单词
                std::string result = _cb(inbuff);//单词
                sendto(_sockfd, result.c_str(), result.size(), 0, (struct sockaddr *)&client_peer, len);//发送给客户端
                // LOG(LogLevel::DEBUG)<<client_address<<inbuff;
                //回显对方信息
                // std::string client_info = "Server say: ";
                // client_info += inbuff;
                // ssize_t size = sendto(_sockfd,client_info.c_str(),client_info.size(),0,(sockaddr*)&client_peer,sizeof(client_peer));
                // if(size<0)
                // {
                //     LOG(LogLevel::WARNING)<<"数据发送失败 请重新发送...";
                // }

            }
            else
            {
                LOG(LogLevel::WARNING)<<"数据接受失败 请重新发送...";
            }
        }
    }

private:
    int _sockfd;
    uint16_t _port;
    callback_t _cb;
    // std::string _ip;
};
