//
// Created by caesar on 2020/1/13.
//

#ifndef TOOLS_POLL_TOOL_H
#define TOOLS_POLL_TOOL_H


#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <cstdio>
#else

#include <poll.h>

#endif

class poll_tool {
private:
    static const char *TAG;
    int *fd = nullptr;
    struct pollfd client{};
public:
    explicit poll_tool(int *fd);

    /**
     * 检查是否有数据
     * @return
     */
    long int check_read_count(int timeout_ms = 1);

    /**
     * 检查是否有数据
     * @return
     */
    static long int check_read_count(int fd, int timeout_ms);
};


#endif //TOOLS_POLL_TOOL_H
