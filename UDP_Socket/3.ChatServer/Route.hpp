#pragma once
#include "UserManager.hpp"
#include "Mutex.hpp"
#include <iostream>
#include <memory>


class Route//整一个流程规律再在管理用户的基础上再封装一层
{
public:
    Route():_uma(std::make_unique<UserManager>())
    {

    }

    void CheckUser(const InetAddr &addr)
    {
        MutexGuard lockguard(_lock);
        _uma->AddUser(addr);
    }

    void OfflineUser(const InetAddr &addr)
    {
        MutexGuard lockguard(_lock);
        _uma->DelUser(addr);
    }

    void Broadcast(int sockfd, std::string message)
    {
        MutexGuard lockguard(_lock);
        auto &users = _uma->Users();
        for(auto &user : users)//向每一个用户都发信息 类似一个聊天室
        {
            //在同一个sockfd中发送给不同人信息---这里就类似一个微信群聊一样
            sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr*)user.GetNetAddress(), user.Len());
        }
    }
    ~Route()
    {

    }
private:
    std::unique_ptr<UserManager> _uma;
    MutexMould _lock;
};