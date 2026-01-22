#include<iostream>
#include<vector>
#include<map>
#include<string>
#include<jsoncpp/json/json.h>
void test1()
{
    Json::Value student;//对象
    student["name"]="liao";
    student["project"].append("math");
    student["project"].append("chinese");
    student["project"].append("English");
    Json::StyledWriter arr;
    //std::cout<<student<<std::endl;
    //std::cout<<arr.write(student)<<std::endl;
    std::string json_string=arr.write(student);//转化为字符
    Json::Reader reader;
    Json::Value value;
    if(!reader.parse(json_string,value))//重新转化为json字符串,反序列化
    {
        std::cerr<<"错误"<<std::endl;
        return ;
    }
    std::string name=value["name"].asString();
    Json::Value p;
    p=value["project"];
    for(int i=0;i<p.size();i++)
    {
        std::cout<<p[i].asString()<<std::endl;
    }
}
void test2()
{
Json::Value root;
root["student"]["name"]="liao";
root["student"]["age"]=20;
Json::Value score;
score["math"]=90;
score["english"]=100;
root["student"]["score"]=score;
Json::StyledWriter w;
std::cout<<w.write(root)<<std::endl;
}
void test3()
{

 std::map<std::string,int>v;
 v["hello"]=10;
 v.emplace("nihao",15);
// Json::Value root;
// root["english"]=v;
for(const auto& pair:v)
{
    
}
}
int main()
{
test1();
test2();
test3();

    return 0;
}