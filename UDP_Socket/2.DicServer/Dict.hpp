#include <iostream>
#include "Logger.hpp"
#include <string>
#include <unordered_map>
#include <fstream>


const std::string default_path = "./dict.txt";
const std::string default_sep = ": ";
using namespace LoggerMould;


class Dict
{
public:
    Dict():_dict_path(default_path)
    {
        LoadDict();
    }
    ~Dict(){}

    void LoadDict()
    {
        std::fstream in(_dict_path);
        if(!in.is_open())
        {
            LOG(LogLevel::ERROR)<<"字典写入失败";
            exit(1);
        }

        std::string line;
        while(std::getline(in,line))//通过观察是否读到文件末尾来判断 字典是否都载入完毕
        {
            LOG(LogLevel::INFO) << "load: " << line << " success";
            auto pos = line.find(default_sep);
            if (pos==std::string::npos)//没有找到放回一个很大的数字
            {
                LOG(LogLevel::ERROR)<< "Format: " << line << " error";
                continue;
            }
            std::string k = line.substr(0,pos);
            std::string v = line.substr(pos+default_sep.size());
            _dict.insert(std::make_pair(k,v));
        }
        in.close();
        LOG(LogLevel::INFO) << "load done....";        
    }

    std::string Translate(std::string word)
    {
        auto iter = _dict.find(word);
        if(iter != _dict.end())
        {
            return "翻译:"+iter->second;
        }
        else{
            return "输入有误,请重新输入";
        }
    }
private:
    std::string _dict_path;
    std::unordered_map<std::string, std::string> _dict;
};

