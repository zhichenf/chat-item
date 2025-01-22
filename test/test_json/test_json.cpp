#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

#include "json.hpp"
using json = nlohmann::json;

void func1() {
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello world";

    cout << js << endl;
}

void func2(){
    json js;
    js["msy_type"] = {{"1","2"}};
    js["msy_type2"] = {"1","2"};
    js["from"]["zhang san"] = "hello";                      //from的值也是一个json对象
    js["from"]["li si"] = "world";

    js["to"] = {{"wang wu","nihao"},{"zhao si","shijie"}}; //to的值是一个json对象
    cout << js << endl;
}

string func3(){
    vector<int> vec = {1,2,3,4,5,6};
    map<int,string> m = {{1,"黄山"},{2,"泰山"}};
    map<int,string> m2 = {{3,"衡山"}};

    json js;
    js["vec"] = vec;            //用容器装的数据会被转化为json中的数组
    js["map"] = m;              //map的值还是一个数组，而不是一个json对象,并且是数组套数组
    js["map2"] = m2;
    cout << js << endl;

    return js.dump();
}

int main() {
    func1();
    func2();
    string recvbuf = func3();

    cout << "json的反序列化" << endl;
    json js = json::parse(recvbuf);
    cout << js["vec"] << endl;
    cout << js["map"] << endl;
    std::vector<int> vec = js["vec"];       //js["vec"].get<std::vector<int>>();
    cout << vec[2] << endl;

    map<int,string> m = js["map"];
    cout << m[1] << endl;
}