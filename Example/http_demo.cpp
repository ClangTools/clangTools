//
// Created by caesar on 2019/12/30.
//

#include <HTTP.h>

using namespace std;

//const char *test_url = "http://kekxv.com/?id=3";
const char *test_url = "http://wap.baidu.com/?id=3";

int main(int argc, char *argv[]) {

    map<string, string> post_data;
    CJsonObject json;
    json.Add("path", "k");
    json.Add("json", "k");
    json.Add("remarks", "k");
    post_data["path"] = "k";
    post_data["json"] = json.ToString();
    post_data["remarks"] = "k";

    HTTP http("http://json.apiopen.top/updateJson");
//    HTTP http("http://127.0.0.1/test.php");
    vector<unsigned char> data;
    http.push_POST(post_data);
    int ret = http.send(data);
    printf("http.send : %d\n", ret);
    if (ret >= 0) {
        data.push_back(0);
        printf("http data : %s \n", data.data());
    }

    HTTP http_get("http://json.apiopen.top/k");
//    HTTP http("http://127.0.0.1/test.php");
    data.clear();
    ret = http_get.send(data);
    printf("http.send : %d\n", ret);
    if (ret >= 0) {
        data.push_back(0);
        printf("http data : %s \n", data.data());
    }


    return 0;
}