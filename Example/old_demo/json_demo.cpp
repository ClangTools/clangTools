//
// Created by caesar on 2019/11/30.
//
#include <CJsonObject.hpp>

using namespace std;

int main(int argc, char *argv[]) {
    string json_str = R"({"code":0,"message":"message"})";
    CJsonObject jsonObject(json_str);
    printf("code\t:%8lld\n", jsonObject["code"].toNumber());
    printf("message\t:%8s\n", jsonObject["message"].toString().c_str());
    jsonObject.Replace("message", "message_change");
    jsonObject.Add("data", "data");
    printf("json\t:\n%s\n", jsonObject.ToFormattedString().c_str());
    printf("end");
    return 0;
}