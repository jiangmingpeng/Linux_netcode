#include "EchoServer_udp.hpp"
#include "Logger.hpp"
#include <iostream>
#include <memory>

int main()
{
    ENABLE_CONSOLE_LOG_STRATEGY()//这里给的是控制台策略

    std::unique_ptr<UdpServer> server = std::make_unique<UdpServer>(8080);
    server->Init();
    server->Start();
    return 0;
}