//
// Created by caesar on 2020/1/15.
//

#include <Popen.h>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
    Popen _popen("cat", std::vector<std::string>{"CMakeCache.txt"});
    string data;
    if (_popen.start())
        if (_popen.read(data) > 0) {
            cout << data << endl;
        }
    return 0;
}