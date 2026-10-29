#include "Calculator.hpp"
#include "Protocol.hpp"
#include "TcpServer.hpp"
#include "Logger.hpp"
#include <iostream>
#include <memory>

static void Usage(const std::string &process)
{
    std::cerr << "Usage:\n\t";
    std::cerr << process << " local_port" << std::endl;
}
// ./server_udp port
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(2);
    }
    ENABLE_CONSOLE_LOG_STRATEGY()//这里给的是控制台策略
    uint16_t server_port = std::stoi(argv[1]);
    //1.业务层
    std::unique_ptr<Calculator> cal = std::make_unique<Calculator>();

    //2.协议层
    std::unique_ptr<Protocol> protocol = std::make_unique<Protocol>
    (
        [&cal](Request &req)->Response
        {
            return cal->Execute(req);
        }
    );
    //3.网络层
    std::unique_ptr<TcpServer> tsvr = std::make_unique<TcpServer>
    (
        [&protocol](std::string& inbuff)->std::string
        {
            return protocol->ParseRequest(inbuff);
        }
        ,server_port
    );

    //4.启动服务
    tsvr->Loop();
    return 0;
}