#include <jsoncpp/json/json.h>
#include <iostream>
#include <string>
#include <sstream>



int main()
{
    // //想要JSON格式的数据这个Value的
    // Json::Value root;//可以单独使用进行转字符串打印使用也可以通过其他不同的
    // root["name"] = "张三";
    // root["age"] = 15;

    // //StreamWriterBuilder - -序列化方法
    // // Json::StreamWriterBuilder Wbuilder;
    // // std::unique_ptr<Json::StreamWriter> writer(Wbuilder.newStreamWriter());
    // // std::stringstream ss;
    // // writer->write(root,&ss);
    // // std::cout<<ss.str()<<std::endl;
    

    // //序列化方法
    // Json::FastWriter writer;
    // std::string root_string = writer.write(root);
    // std::cout<<root_string<<std::endl;
    
    // 反序列化
    // 反序列化依然拥有很多变种
    std::string string_root =  "{\"name\":\"张三\", \"age\":30,\"city\":\"北京\"}";

    Json::Value root;
    Json::Reader reader;
    reader.parse(string_root,root);

    std::string name = root["name"].asString();
    int age = root["age"].asInt();

    std::cout<<name<<std::endl<<age<<std::endl;

    

    // std::string root_string = root.toStyledString();
    // 
    // std::cout<<root_string<<std::endl;

    return 0;
}