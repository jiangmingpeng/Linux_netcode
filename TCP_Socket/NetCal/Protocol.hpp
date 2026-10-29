#pragma once
#include <iostream>
#include <jsoncpp/json/json.h>
#include <string>
#include <functional>

#include "Logger.hpp"

using namespace LoggerMould;

// 请求报文
class Request
{
public:
    Request()
        : _data_x(0), _data_y(0), _oper(0)
    {
    }
    Request(int x,int y,char oper)
        : _data_x(x), _data_y(y), _oper(oper)
    {
    }
    ~Request()
    {
    }

    bool Serialize(std::string *out)
    {
        Json::Value root;
        root["_data_x"] = _data_x;
        root["_data_y"] = _data_y;
        root["_oper"] = _oper;

        Json::FastWriter writer;
        *out = writer.write(root);
        return true;
    }

    bool Deserialize(std::string &in)
    {
        Json::Value root;
        Json::Reader reader;
        bool desri = reader.parse(in, root);
        if (!desri)
        {
            LOG(LogLevel::WARNING) << "Deserialize fail...";
            return false;
        }

        _data_x = root["_data_x"].asInt();
        _data_y = root["_data_y"].asInt();
        _oper = root["_oper"].asInt();

        return true;
    }

public:
    int _data_x;
    int _data_y;
    char _oper; // '+' '-' '/' '*' '%'
};

// 回应报文
class Response
{
public:
    Response()
        : _result(0), _code(0)
    {
    }
    Response(int result, int code)
        : _result(result), _code(code)
    {
    }
    ~Response()
    {
    }

    bool Serialize(std::string *out)
    {
        Json::Value root;
        root["result"] = _result;
        root["code"] = _code;

        Json::FastWriter writer;
        *out = writer.write(root);
        return true;
    }

    bool Deserialize(std::string &in)
    {
        Json::Value root;
        Json::Reader reader;
        bool desri = reader.parse(in, root);
        if (!desri)
        {
            LOG(LogLevel::WARNING) << "Deserialize fail...";
            return false;
        }

        _result = root["result"].asInt();
        _code = root["code"].asInt();

        return true;
    }

public:
    int _result; // 结果
    int _code;   // 状态码
};

const std::string gsep = "\r\n";

using HandlerRequest_t = std::function<Response(Request &)>;
using HandlerResponse_t = std::function<void(Response &)>;

// 处理协议
// len\r\n{"left": 10, "right": 20, oper: '+'}\r\n
class Protocol
{
public:
    Protocol(HandlerRequest_t handler) : _version("1.0"), _handler_request(handler)
    {
    }
    Protocol(HandlerResponse_t handler_response) : _version("1.0"), _handler_response(handler_response)
    {
    }
    ~Protocol()
    {
    }

    // {"left": 10, "right": 20, oper: '+'}
    // len\r\n{"left": 10, "right": 20, oper: '+'}\r\n

    std::string Packet(const std::string &json_string)
    {
        return std::to_string(json_string.size()) + gsep + json_string + gsep;
    }
    // len\r\n{"left": 10, "right": 20, oper: '+'}\r\n
    // len\r\n{"left": 10, "right": 20, oper: '+'}\r\nlen\r\n{"left": 10, "right": 20, oper: '+'}\r\n
    // len\r\n{"left": 10, "right": 20, oper: '+'}\r\nlen\r\n{"left": 10,
    // len\r\n{"left": 10, "right": 20, o
    // le
    // ret > 0: no error, json_string != NULL
    // ret == 0: no error, json_string == NULL
    // ret < 0 : error.
    int Unpack(std::string &packet, std::string *json_string)
    {
        if (packet.empty())
            return 0;
        if (json_string == nullptr)
            return -1;

        auto pos = packet.find(gsep);
        if (pos == std::string::npos)
            return 0;
        // 报文大小拿出来
        std::string lenstr = packet.substr(0, pos);

        int len = std::stoi(lenstr); // 报文正文的长度
        int total = lenstr.size() + len + 2 * gsep.size();
        if (packet.size() < total)
            return 0;
        // 提取报文
        *json_string = packet.substr(pos + gsep.size(), len);
        packet.erase(0, total);
        return 1;
    }

    std::string ParseRequest(std::string &inbuff)
    {
        std::string result;
        while (true)
        {
            std::string json_string;
            // 1.解包
            int n = Unpack(inbuff, &json_string);
            if (n < 0)
            {
                LOG(LogLevel::WARNING) << "Unpack fail ..";
                return std::string();
            }
            if (n == 0)
            {
                LOG(LogLevel::INFO) << inbuff << " parse done";
                return result;
            }
            LOG(LogLevel::DEBUG) << "After Unpack json_string: " << json_string;
            LOG(LogLevel::DEBUG) << "unpack done, inbuffer:\n"
                                 << inbuff;

            // 2.反序列化
            Request req;
            if (!req.Deserialize(json_string))
                return std::string();

            // 3.处理业务
            Response resp;
            if (_handler_request)
            {
                resp = _handler_request(req);
            }

            // 4. 应答序列化
            std::string resp_json_string;
            resp.Serialize(&resp_json_string);

            // 5. 添加报头
            result += Packet(resp_json_string);
        }
    }
    std::string ParseResponse(std::string &inbuff)//把解析结果弄出去
    {
        while (true)
        {
            std::string json_string;
            // 1.解包
            int n = Unpack(inbuff, &json_string);
            if (n < 0)
            {
                LOG(LogLevel::WARNING) << "Unpack fail ..";
                return std::string();
            }
            if (n == 0) return std::string();
            LOG(LogLevel::DEBUG) << "After Unpack json_string: " << json_string;
            LOG(LogLevel::DEBUG) << "unpack done, inbuffer:\n"
                                 << inbuff;
            // 2.反序列化
            Response resp;
            if (!resp.Deserialize(json_string))
                return std::string();
            // 3.通过回调处理回复
            if (_handler_response)
            {
                _handler_response(resp);
            }
        }
    }

private:
    std::string _version;
    HandlerRequest_t _handler_request;
    HandlerResponse_t _handler_response;
};
