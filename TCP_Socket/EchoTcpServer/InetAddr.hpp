#pragma once 
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>


class InetAddr//客户端地址描述
{
public:
    InetAddr(const struct sockaddr_in &address):_address(address), _len(sizeof(address))
    {
        _ip = inet_ntoa(_address.sin_addr);//given in network byte order, to a string in IPv4 dotted-decimal
        _port = ntohs(_address.sin_port);
    }

    InetAddr(uint16_t port,std::string ip = "0.0.0.0"):_port(port),_ip(ip)
    {
        memset(&_address,0,sizeof(_address));
        _address.sin_family = AF_INET;
        _address.sin_port = htons(_port);
        _address.sin_addr.s_addr = inet_addr(_ip.c_str()); // 1. 字符串ip->4字节IP 2. hton 
        _len = sizeof(_address);       
    }

    bool operator == (const InetAddr &addr)
    {
        return (this->_ip == addr._ip) && (this->_port == addr._port);
    }
    std::string ToString()//用来标记是谁
    {
        return "[" + _ip + ":" + std::to_string(_port) + "]";
    }
    InetAddr()
    {}
    struct sockaddr_in* GetNetAddress()
    {
        return &_address;
    }
    socklen_t Len()
    {
        return _len;
    }

    ~InetAddr()
    {

    }
private:
    // net address
    struct sockaddr_in _address;
    socklen_t _len;
    // host address
    std::string _ip;
    uint16_t _port;
};