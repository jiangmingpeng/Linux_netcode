#include "Logger.hpp"
#include "InetAddr.hpp"
#include "Thread.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

using namespace LoggerMould;
using namespace ThreadMould;

int sockfd = 0;
std::string server_ip;
uint16_t server_port = 0;
std::string nickname;

static void Usage(const std::string &proc)
{
    std::cout << "Usage:\n\t";
    std::cout << proc << " server_ip server_port" << std::endl;
}

void Online(InetAddr& serveraddr)
{
    std::cout << "Please Set Your Nick Name# ";
    std::getline(std::cin, nickname);
    std::string online_message = nickname + " online!";
    ssize_t n = sendto(sockfd, online_message.c_str(), online_message.size(), 0,
                       (struct sockaddr *)serveraddr.GetNetAddress(), serveraddr.Len());
    (void)n;

}

void RecvMessage()
{
    while (true)
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
}

void SendMessage()
{
    InetAddr serveraddr(server_port,server_ip);
    Online(serveraddr);
    // 3.接受用户输入信息 并且发送给服务端
    while (true)
    {
        // 接受用户输入
        std::string message;
        std::cout<<"Please Enter# ";
        std::getline(std::cin, message);
        message = nickname + "#" + message;
        // 向服务端发送信息
        ssize_t n = sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)serveraddr.GetNetAddress(), serveraddr.Len());
        (void)n;
    }
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        Usage(argv[0]);
        exit(1);
    }

    server_ip = argv[1];
    server_port = std::stoi(argv[2]);

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "创建套接字文件失败..." << std::endl;
    }

    Thread recv(RecvMessage); 
    Thread send(SendMessage); 

    recv.Start();
    send.Start();

    recv.Join();
    send.Join();

    return 0;
}
