//
// Created by caesar on 2020/1/15.
//

#ifndef TOOLS_POPEN_H
#define TOOLS_POPEN_H

#ifdef _WIN32
#include <Windows.h>
#else

#include <zconf.h>

#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>


class Popen {
private:
    static const char *TAG;
private:
    FILE *fp = nullptr;
    int fd = 0;
    std::string command_line;
public:
    Popen(const char *command, const std::vector<std::string> &args);

    ~Popen();

    bool start();

    bool stop();

    int read(std::string &data, int timeout = 3000);

    int write(const std::string& data);
};


#endif //TOOLS_POPEN_H
