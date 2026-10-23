#pragma once
#include <cstdio>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pwd.h>
#include <string>
#include <vector>

class Command
{
private:
    void LoadCommand()
    {
        _whitelist.push_back("ls -a -l");
        _whitelist.push_back("ls -l");
        _whitelist.push_back("pwd");
        _whitelist.push_back("whoami");
        _whitelist.push_back("who");
        _whitelist.push_back("ps -al");
        _whitelist.push_back("netstat -nltp");
        _whitelist.push_back("netstat -nutp");
        _whitelist.push_back("netstat -natp");
        _whitelist.push_back("netstat -naup");
    }
    
    bool IsSafe(std::string str)
    {
        for(auto e : _whitelist)
        {
            if(e==str)
            {
                return true;
            }
        }
        return false;
    }

public:
    Command(){LoadCommand();}
    ~Command(){}

    std::string Excute(std::string& cmd)
    {
        if(!IsSafe(cmd))
        {
            return "别乱输入命令 一边去！！";
        }

        FILE* fp = popen(cmd.c_str(),"r");
        if(fp == nullptr)
        {
            return "执行有误";
            // exit(3);
        }
        std::string result;
        char inbuff[1024];
        while(fgets(inbuff,sizeof(inbuff),fp))
        {
            result+=inbuff;
        }

        pclose(fp);
        return result;
    }
    
    std::string GetString()
    {
        // JMP@iZuf6f1eoyk5xd64cbk0w8Z:~$
        struct passwd* pw = getpwuid(getuid());
        std::string usrname = (pw?pw->pw_name : "unknown");

        char hostname[256] ;
        if(!gethostname(hostname,sizeof(hostname)))
        {
            snprintf(hostname,sizeof(hostname),"unknown");
        } 

        char cwd[1024];
        if(!getcwd(cwd,sizeof(cwd)))
        {
            snprintf(cwd,sizeof(cwd),"unknown");
        }

        return usrname+"@"+hostname+":"+cwd+"# ";
    }

private:
    std::vector<std::string> _whitelist;
};