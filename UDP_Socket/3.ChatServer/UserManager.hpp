#pragma once 
#include <iostream>
#include <vector>
#include "InetAddr.hpp"


class UserManager
{
public:
    UserManager()
    {

    }
    ~UserManager()
    {

    }
    void AddUser(const InetAddr &addr)
    {
        if(SearchUser(addr))
        {
            return; 
        }
        _users.emplace_back(addr);
    }
    void DelUser(const InetAddr &addr)
    {
        auto iter = _users.begin();
        for(;iter != _users.end();iter++)
        {
            if(*iter == addr)
            {
                _users.erase(iter);
                break;
            }
        }
        return;
    }
    bool SearchUser(const InetAddr &addr)
    {
        for(auto &user : _users)
        {
            if(user == addr)
            {
                return true;
            }
        }
        return false;
    }
    bool ModUser(const InetAddr &addr)
    {
        // 简单一点
        DelUser(addr);
        AddUser(addr);
        return true;
    }
    std::vector<InetAddr> &Users()//把用户数据通过对象返回出去
    {
        return _users;
    }
private:
    std::vector<InetAddr> _users;
};