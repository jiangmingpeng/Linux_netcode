#include "EchoServer_udp.hpp"
#include "Dict.hpp"
#include "Logger.hpp"
#include <iostream>
#include <memory>
static void Usage(const std::string &process)
{
    std::cerr << "Usage:\n\t";
    std::cerr << process << " local_port" << std::endl;
}

// ./server_udp port---可以自己自定义端口号也可以 使用默认端口号
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }
    
    ENABLE_CONSOLE_LOG_STRATEGY()//这里给的是控制台策略

    // 1. 定义字典
    Dict dict;
    // 2. 构建网络服务，处理IO问题
    uint16_t server_port = std::stoi(argv[1]);
    // 3. 绑定上下两层
    std::unique_ptr<UdpServer> usvr = std::make_unique<UdpServer>([&dict](std::string word)->std::string
    {
        return dict.Translate(word);
    }
    , server_port);

    // std::unique_ptr<UdpServer> server = std::make_unique<UdpServer>(8080);
    usvr->Init();
    usvr->Start();
    return 0;
}