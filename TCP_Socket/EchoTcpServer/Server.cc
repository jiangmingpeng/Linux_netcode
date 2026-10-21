#include "EchoServer_tcp.hpp"
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

    std::unique_ptr<TcpServer> server = std::make_unique<TcpServer>(server_port);
    server->Init();
    server->Start();
    return 0;
}