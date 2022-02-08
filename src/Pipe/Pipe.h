//
// Created by caesar on 2020/1/6.
//

#ifndef TOOLS_PIPE_H
#define TOOLS_PIPE_H
#include <string>
#include <vector>

#ifdef WIN32
#include <cstdio>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#else

#include <poll.h>

#endif

class Pipe {
private:
    static const char *TAG;
    bool is_parent = false;
    int fd_R = 0;
    int fd_W = 0;
    std::string pipe_name;
    const int max_len = 2048;
    struct pollfd client{};
public:
    explicit Pipe(const std::string &name);

    ~Pipe();

    int Init(bool _is_parent);

    void Free();

    int check_read_count(int timeout_ms = 1);

    int read(std::vector<unsigned char> &data);

    int send(const std::string &data);

    int send(std::vector<unsigned char> data);

    int send(void *data, int len);

};


#endif //TOOLS_PIPE_H
