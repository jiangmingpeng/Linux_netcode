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


    // 4.接受用户输入信息 并且发送给服务端
    while (true)
    {
        // 接受用户输入
        std::string message;
        std::cout<<"say#";
        std::getline(std::cin, message);

        // 向服务端发送信息
        ssize_t num = write(sockfd,message.c_str(),message.size());
        if(num < 0)
        {
            LOG(LogLevel::WARNING)<<"write fail....";
            continue;
        }

        // 接受服务端的回显
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
