#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Logger.hpp"
#include "InetAddr.hpp"

using namespace LoggerMould;

static void Usage(const std::string &proc)
{
    std::cout << "Usage:\n\t";
    std::cout << proc << " server_ip server_port" << std::endl;
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }

    std::string server_ip = argv[1];
    int server_port = std::stoi(argv[2]);

    // 1.创建套接字--面向字节流-tcp
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "创建套接字文件失败..." << std::endl;
    }

    // 2.写入服务端基本信息 不用绑定固定IP随机即可 
    InetAddr server(server_port,server_ip);
    // 3.进行建立tcp连接
    int n = connect(sockfd, (struct sockaddr *)server.GetNetAddress(), server.Len());
    if (n < 0)
    {
        std::cerr << "connect to " << server.ToString() << " failed!"<< std::endl;
        exit(3);
    }
    std::cerr << "connect to " << server.ToString() << " success!" << std::endl;


    // 4.接受用户输入命令 并且发送给服务端进行执行回显结果
    while (true)
    {
        char commandline[1024];
        std::string commandlinestr;
        ssize_t n = read(sockfd,commandline,sizeof(commandline)-1);
        if(n>0)
        {
            commandline[n] = 0;
            commandlinestr = commandline;
        }
        else{
            commandlinestr = "unknow@unknown None:# ";
        }
        

        std::cout<<commandlinestr;

        // 处理用户输入的命令
        std::string command;
        std::getline(std::cin, command);
        // 向服务端发送
        ssize_t num = write(sockfd,command.c_str(),command.size());
        if(num < 0)
        {
            LOG(LogLevel::WARNING)<<"ssh fail....";
            continue;
        }

        // 接受服务端的回显(既然是回显输出信息这里复用即可)
        char inbuff[1024] ;
        n = read(sockfd,inbuff,sizeof(inbuff)-1);
        if(n>0)
        {
            inbuff[n] = '\0';
            std::cout<<inbuff<<std::endl;
        }
        else if(n == 0)
        {
            std::cout<<"read end of file.."<<std::endl;
            break;
        }
        else
        {
            std::cout<<"read end of file.."<<std::endl;
            break;
        }
    }

    return 0;
}
