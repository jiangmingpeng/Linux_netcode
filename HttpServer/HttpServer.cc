#include "TcpServer.hpp"
#include "Logger.hpp"
#include "Http.hpp"
#include "daemon.hpp"
#include <iostream>
#include <memory>

static void Usage(const std::string &process)
{
    std::cerr << "Usage:\n\t";
    std::cerr << process << " local_port" << std::endl;
}

// 从 req.Text() 里取字段：GET 是 ?user=x&pass=y，POST 是表单正文，两者格式一样
static std::string GetArg(HttpRequest &req, const std::string &key)
{
    std::string args = req.Text();
    std::string pat = key + "=";
    size_t pos = args.find(pat);
    if (pos == std::string::npos)
        return "";
    pos += pat.size();
    size_t end = args.find('&', pos);
    return args.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

void Login(HttpRequest &req, HttpResponse &rep)
{
    const std::string user = GetArg(req, "user");
    const std::string pass = GetArg(req, "pass");

    const bool ok = (user == "admin" && pass == "123456"); // TODO: 换成自己的校验

    std::string body = ok ? "<h2>Login seccess !!!" + user + "</h2>"
                          : "<h2>usr is erro!!!</h2>";

    rep.SetCode(ok ? 200 : 403);
    rep.SetBody(body);
    rep.AddHeader("Content-Length", std::to_string(body.size()));
    rep.AddHeader("Content-Type", "text/html");
    rep.AddHeader("Connection", "close");

    LOG(LogLevel::INFO) << "Login: " << user << " " << (ok ? "ok" : "fail");
}

void Register(HttpRequest &req, HttpResponse &rep)
{
    const std::string user    = GetArg(req, "user");
    const std::string pass    = GetArg(req, "pass");
    const std::string confirm = GetArg(req, "confirm");      // 对应 CONFIRM_PASS

    int code = 200;
    std::string body;

    if (user.empty() || pass.empty())   { code = 400; body = "<h2>参数不完整</h2>"; }
    else if (pass != confirm)           { code = 400; body = "<h2>两次密码不一致</h2>"; }
    else                                { body = "<h2>注册成功：" + user + "</h2>"; }

    rep.SetCode(code);
    rep.SetBody(body);
    rep.AddHeader("Content-Length", std::to_string(body.size()));
    rep.AddHeader("Content-Type", "text/html");
    rep.AddHeader("Connection", "close");

    LOG(LogLevel::INFO) << "Register: " << user << " code=" << code;
}

void Search(HttpRequest &req, HttpResponse &rep)
{
    std::cout << "Search..." << std::endl;
}

// ./server_udp port
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(2);
    }
    ENABLE_FILE_LOG_STRATEGY();
    // ENABLE_CONSOLE_LOG_STRATEGY();
    uint16_t server_port = std::stoi(argv[1]);

    Daemon(1, 0); 
    // daemon(0, 0);
    // LOG -> 文件版日志
    // printf && cout -> /dev/null

    // 定义Http协议
    std::unique_ptr<HttpProtocol> protocol = std::make_unique<HttpProtocol>();
    protocol->RegisterService("/Login", Login);
    protocol->RegisterService("/Register", Register);
    protocol->RegisterService("/Search", Search);

    // 3.网络层
    std::unique_ptr<TcpServer> tsvr = std::make_unique<TcpServer>(
        [&protocol](std::string &inbuff) -> std::string
        {
            return protocol->HanlderHttpRequest(inbuff);
        },
        server_port);

    // 4.启动服务
    tsvr->Loop();
    return 0;
}