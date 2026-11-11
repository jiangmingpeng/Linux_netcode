#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <functional>
#include <strings.h>
#include "Logger.hpp"

using namespace LoggerMould;

const std::string linesep = "\r\n";
const std::string spacesep = " ";
const std::string headersep = ": ";
const std::string g_wwwroot = "wwwroot";
const std::string g_httpversion = "HTTP/1.0";
const std::string g_first_page = "index.html"; //浏览器对web根目录发起请求 这个根目录uri首次显示的页面由提供服务的地方手动设置
const std::string page_404 = "./wwwroot/404.html";
const std::string suffixsep = ".";
const std::string argsep = "?"; // uri和参数的分隔符

class Util
{
public:
    static std::string ReadLine(std::string &str) // 读取字符以行为单位 \r\n为一行
    {
        int pos = str.find(linesep);
        if (pos == std::string::npos)
        {
            return std::string();
        }
        std::string line = str.substr(0, pos);
        // 把截取之后的串进行擦除
        str.erase(0, line.size() + linesep.size());
        if (line.empty())
            return linesep;
        return line;
    }

    static std::string ReadFile(const std::string &filename) // 这个传过来的文件名字就是uri所指定的文件的保存路径 读文件的方法
    {

        // 以二进制打开所需要访问的文件  文本文件依然是可以通过二进制的方式进行访问的
        std::ifstream in(filename, std::ios::binary);
        if (!in.is_open())
        {
            LOG(LogLevel::ERROR) << "open file fail: [" << filename
                                 << "] len=" << filename.size();
            return std::string();
        }

        // 得到二进制文件的大小
        in.seekg(0, in.end);
        int filesize = in.tellg();
        in.seekg(0, in.beg);

        // 进行文件内容读取
        std::string content;
        content.resize(filesize);
        in.read((char *)content.c_str(), filesize);
        in.close();
        return content;

        // 以普通txt格式的样式进行文件访问
        //  // 读取一个纯字符串的文件 这里并未考虑其他文件格式
        //  std::fstream in(filename); // 把文件里的数据拿到流里面去
        //  if (!in.is_open())
        //  {
        //      LOG(LogLevel::FATAL) << "open filename...";
        //      return std::string();
        //  }
        //  std::string line;
        //  std::string content;
        //  while (std::getline(in, line))
        //  {
        //      content += line;
        //  }
        //  return content;
    }
};

class HttpRequest
{
private:
    bool ParseReqLine(std::string &httpstr)
    {

        std::string req_line = Util::ReadLine(httpstr);
        if (req_line.empty() || req_line == linesep)
            return false;
        std::stringstream ss(req_line);
        ss >> _method >> _uri >> _version;

        // 处理一下uri的问题
        if (_uri == "/")
        {
            _uri += g_first_page; // uri: /-> uri: /index.html
        }
        _uri = g_wwwroot + _uri; // wwwroot/index.html: web 根目录
        // wwwroot/Login
        // wwwroot/index.html
        // wwwroot/index.css
        // wwwroot/index.js
        // wwwroot/image/xxx.jpg,png...
        // wwwroot/video/xxx.mpx

        // 进行后缀处理
        int pos = _uri.rfind(suffixsep);
        if (pos != std::string::npos)
                _suffix = _uri.substr(pos);

        return true;
    }

    bool ParseHeaderkv(std::string &httpstr)
    {
        std::string req_header;
        do
        {
            req_header = Util::ReadLine(httpstr);
            if (req_header.empty())
                return false;
            if (req_header != linesep)
            {
                int pos = req_header.find(headersep);
                if (pos == std::string::npos)
                    return false;

                std::string k = req_header.substr(0, pos);
                std::string v = req_header.substr(pos + headersep.size());

                _headers.insert(std::make_pair(k, v));
            }
        } while (req_header != linesep);
        return true;
    }

    bool ParseText(std::string &httpstr) // 请求正文的处理-->为反序列化做准备
    {
        // 1.有没有正文 结合发起请求的方法//找？ 判断是否传参
        if (strcasecmp(_method.c_str(), "GET") == 0)//忽略大小写进行相等判断
        {
            auto pos = _uri.find(argsep);
            if(pos ==std::string::npos)
            {
                _text = std::string();
                return true;
            }
            else
            {
                std::cout<<"_URI :"<<_uri<<std::endl;
                std::string temp = _uri;
                _uri = temp.substr(0,pos);
                std::cout<<"_URI :"<<_uri<<std::endl;
                _text = temp.substr(pos+argsep.size());
                std::cout<<"_text :"<<_text<<std::endl;
            }
        }
        else
        {
            // 处理正文 先进行报头检测如果连content—lenth这个都没有那正文长度等于""就是很正常的事
            if (_headers.find("Content-Length") == _headers.end()) // 没找到长度报头的情况
            {
                _text = "";
                return true;
            }
            // 2. 正文多大？从哪开始？
            // POST请求方法
            int content_len = std::stoi(_headers["Content-Length"]);
            _text = httpstr.substr(0, content_len);
            httpstr.erase(0, content_len);
        }

        return true;
    }

public:
    HttpRequest()
    {
    }
    ~HttpRequest()
    {
    }
    // 反序列化
    bool Deserialize(std::string &htttpstr)
    {
        // 1.解析请求行
        int n = ParseReqLine(htttpstr);
        if (!n)
        {
            LOG(LogLevel::FATAL) << "ParseReqLine fail..";
            return false;
        }
        // 2.解析请求报头
        n = ParseHeaderkv(htttpstr);
        if (!n)
        {
            LOG(LogLevel::FATAL) << "ParseHeaderkv fail..";
            return false;
        }
        // 3.解析空行
        _blank_line = linesep;
        // 4.解析正文
        n = ParseText(htttpstr);
        if (!n)
        {
            LOG(LogLevel::FATAL) << "ParseText fail..";
            return false;
        }

        return true;
    }
    // 解析uri
    std::string Uri()
    {
        return _uri;
    }
    std::string RequestContent()//读的文件就是在该工作目录下自己指定的一些文件 可以是 图片 视频 音乐 前端等其他东西
    {
        std::string path = _uri;          // wwwroot/Login  |  wwwroot/index.html
        if (_suffix.empty())              // 没有后缀 → 按原来的行为补 .html
            path += ".html";        
        return Util::ReadFile(path); // 请求内容最后以读到的二进制文件内容放回
    }
    std::string Text()
    {
        return _text;
    }

    std::string Suffix()
    {
        return _suffix;
    }

private:
    std::string _method;
    std::string _uri;
    std::string _version;
    std::unordered_map<std::string, std::string> _headers;
    std::string _blank_line;
    std::string _text;

    // 文件后缀名
    std::string _suffix;
};

class HttpResponse
{
private:
    std::string CodeDesc(int code)
    {
        switch (code)
        {
        // 1xx
        case 100:
            return "Continue";
        case 101:
            return "Switching Protocols";
        // 2xx
        case 200:
            return "OK";
        case 201:
            return "Created";
        case 202:
            return "Accepted";
        case 204:
            return "No Content";
        case 206:
            return "Partial Content";
        // 3xx
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 304:
            return "Not Modified";
        case 307:
            return "Temporary Redirect";
        case 308:
            return "Permanent Redirect";
        // 4xx
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 405:
            return "Method Not Allowed";
        case 408:
            return "Request Timeout";
        case 409:
            return "Conflict";
        case 413:
            return "Payload Too Large";
        case 429:
            return "Too Many Requests";
        // 5xx
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        case 504:
            return "Gateway Timeout";
        case 505:
            return "HTTP Version Not Supported";
        // 默认情况
        default:
            return "Unknown Status Code";
        }
    }

public:
    void SetCode(int code)
    {
        _code = code;
        _code_desc = CodeDesc(code);
    }

    void SetBody(std::string &content)
    {
        _text = content;
    }

    int BodySize()
    {
        return _text.size();
    }

    // 添加所需报头
    void AddHeader(const std::string &key, const std::string &value)
    {
        _headers[key] = value;
    }

    // 序列化
    std::string Serialize()
    {
        std::string rep_str = _version;
        rep_str += spacesep;
        rep_str += std::to_string(_code);
        rep_str += spacesep;
        rep_str += _code_desc;
        rep_str += linesep;

        // 添加报头
        for (auto &header : _headers)
        {
            std::string line = header.first + headersep + header.second + linesep;
            rep_str += line;
        }

        // 空行和正文
        rep_str += _blank_line;
        rep_str += _text;

        return rep_str;
    }

public:
    HttpResponse() : _version(g_httpversion), _code(0), _blank_line(linesep)
    {
    }
    ~HttpResponse()
    {
    }

private:
    std::string _version;
    int _code;
    std::string _code_desc;
    std::unordered_map<std::string, std::string> _headers;
    std::string _blank_line;
    std::string _text;
};

using service_t = std::function<void(HttpRequest &req, HttpResponse &rep)>; // 服务列表 例如登录 注册这些服务 要对应到链接发生的变化上
class MimeType
{
public:
    static std::string Suffix2MimeType(const std::string &suffix)
    {
        auto iter = _mime_map.find(suffix);
        if (iter != _mime_map.end()) // 找到了
        {
            return iter->second;
        }
        return "text/html";
    }
    static std::unordered_map<std::string, std::string> _mime_map;
};

std::unordered_map<std::string, std::string> MimeType::_mime_map =
    {
        // 文本类型
        {".txt", "text/plain"},
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},

        // 图片类型
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".png", "image/png"},
        {".gif", "image/gif"},

        // 应用类型
        {".pdf", "application/pdf"},
        {".json", "application/json"},
        {".xml", "application/xml"},

        // 脚本类型
        {".js", "application/javascript"},

        // 其他常用类型
        {".zip", "application/zip"},
        {".mp3", "audio/mpeg"},
        {".mp4", "video/mp4"}}; // 不同文件后缀在浏览器解释的时候翻译成文件扩展名

class HttpProtocol
{
public:
    HttpProtocol()
    {
    }
    ~HttpProtocol()
    {
    }

    void RegisterService(const std::string &uri, service_t service) // 传参注册服务
    {
        std::string key = g_wwwroot + uri; // 注意这个uri要手动加上根目录
        _http_services[key] = service;     // key -> wwwroot/login or wwwroot/register
    }

    bool IsReqService(const std::string &uri) // 遍历哈希表判断是否注册这个服务
    {
        auto iter = _http_services.find(uri);
        if (iter == _http_services.end())
            return false;
        return true;
    }

public:
    std::string HanlderHttpRequest(std::string &req) // 这个参数是请求的原始字符串型式  ---这个内容是inbuff里传来的 其实就是从网络里拿到的http请求 放到这里来做解析
    {

#ifndef DEBUG // 条件编译可以熟悉一下 感受一下条件编译的好处 在这里知道有这个功能就可以 不必要在这里再次模仿浪费时间
        // 反序列化
        HttpRequest http_req;
        http_req.Deserialize(req);
        HttpResponse http_rep;

        if (IsReqService(http_req.Uri())) // 动态--对于像注册 登录这种服务
        {
            _http_services[http_req.Uri()](http_req, http_rep);
        }
        else // 静态
        {
            std::string content = http_req.RequestContent();//就是以二进制文件读来的内容
            if (content.empty())
            {
                std::string body = Util::ReadFile(page_404); // 需要把常量改成 "./wwwroot/page_404.html"
                http_rep.SetCode(404);
                http_rep.SetBody(body);
                http_rep.AddHeader("Content-Length", std::to_string(body.size()));
                http_rep.AddHeader("Content-Type", "text/html");
                http_rep.AddHeader("Connection", "close");
            }
            else
            {
                http_rep.SetCode(200);
                http_rep.SetBody(content);
                http_rep.AddHeader("Cookie","usr=123456;");//添加cookie
                http_rep.AddHeader("Content-Length", std::to_string(content.size()));
                http_rep.AddHeader("Content-Type", MimeType::Suffix2MimeType(http_req.Suffix()));
                http_rep.AddHeader("Connection", "close");
            }
        }
        // 序列化返回
        return http_rep.Serialize();
#endif
    }

private:
    std::unordered_map<std::string, service_t> _http_services; // 根据uri(服务路径)进行功能路由
};