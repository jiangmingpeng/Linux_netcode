#include "Logger.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

    // 1.创建套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "创建套接字文件失败..." << std::endl;
    }

    // 2.写入基本信息 不用绑定固定IP随机即可 sendto的时候会根据提供信息自动绑定
    struct sockaddr_in server; // 注意这里填的是要发送的提供服务的主机信息
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(server_port); // 这个端口是要从主机发送到网络里去 要注意网络字节序的修正
    server.sin_addr.s_addr = inet_addr(server_ip.c_str());

    // 3.接受用户输入信息 并且发送给服务端
    while (true)
    {
        // 接受用户输入
        std::string message;
        std::cout<<"say#";
        std::getline(std::cin, message);

        // 向服务端发送信息
        ssize_t n = sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&server, sizeof(server));
        if (n > 0)
        {
            char inbuff[1024] = {0};
            struct sockaddr_in temp;
            socklen_t len = sizeof(temp);
            ssize_t size = recvfrom(sockfd, inbuff, sizeof(inbuff)-1, 0, (struct sockaddr *)&temp, &len);
            if (size > 0)
            {
                // 把回显字符串 用\0分隔开来
                inbuff[size] = '\0';
                std::cout << inbuff << std::endl;
            }
        }
        else
        {
            std::cerr << "内容发送失败 请重新发送.... " << std::endl;
        }

        // 接受服务端的回显
    }

    return 0;
}
