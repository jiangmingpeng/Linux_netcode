#include "ThreadPool.hpp"
#include "Route.hpp"
#include "ChatServer_udp.hpp"
#include "Logger.hpp"
#include <iostream>
#include <memory>
#include <functional>

static void Usage(const std::string &process)
{
    std::cerr << "Usage:\n\t";
    std::cerr << process << " local_port" << std::endl;
}


using namespace ThreadPoolMould;

using task_t = std::function<void(void)>;


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

    //创建线程池
    auto thread_pool = ThreadPool<task_t>::Instance();    

    //路由服务
    Route r;

    //网络服务
    UdpServer usvr(server_port);
    usvr.Init();

    usvr.Register
    (
        [&r](const InetAddr &addr)
        {
            r.CheckUser(addr);
        },
        [&r,thread_pool](int sockfd,std::string msg)
        {
            auto t = std::bind(&Route::Broadcast,&r,sockfd,msg);//在这里t是一个任务,&r是this参数 后面是要传的参数
            thread_pool->Push(t);
        }
    );

    usvr.Start();
    return 0;
}