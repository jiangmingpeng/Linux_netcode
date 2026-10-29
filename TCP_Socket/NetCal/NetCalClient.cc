#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Logger.hpp"
#include "InetAddr.hpp"
#include "Socket.hpp"
#include "Protocol.hpp"

using namespace LoggerMould;
using namespace SOCKETMOULD;

static void Usage(const std::string &proc)
{
    std::cout << "Usage:\n\t";
    std::cout << proc << " server_ip server_port" << std::endl;
}

static void HanlderResponse(Response &resp)
{
    std::cout << "result: " << resp._result << "[" << resp._code << "]" << std::endl;
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

    //建立连接
    std::unique_ptr<Socket> socket = std::make_unique<TcpSocket>();    
    socket->BuildTcpClientSockMethod();
    InetAddr serveraddress(server_port, server_ip);
    bool n = socket->Connect(serveraddress);
    if(!n)
    {
        LOG(LogLevel::WARNING)<<"socket->Connect fail...";
        exit(1);
    }
    
    //
    Protocol protocol(HanlderResponse);
    std::string inbuff;
    while(true)
    {   
        //获取数据
        int x,y;
        char oper;
        std::cout<<"Please Enter your x: ";
        std::cin>>x;
        std::cout<<"Please Enter your y: ";
        std::cin>>y;
        std::cout<<"Please Enter your oper: ";
        std::cin>>oper;

        //定义请求对象
        Request req(x,y,oper);

        //序列化
        std::string req_json;
        req.Serialize(&req_json);
        
        //封装报头
        std::string send_req_string = protocol.Packet(req_json);
        
        //发送
        socket->Send(send_req_string);
        
        //接收
        socket->Recv(&inbuff);
        
        //解析
        protocol.ParseResponse(inbuff);
    }
    socket->Close();
    return 0;
}