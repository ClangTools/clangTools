//
// Created by caesar on 2020/1/13.
//

#ifndef TOOLS_POLL_TOOL_H
#define TOOLS_POLL_TOOL_H


#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
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
    poll_tool(int *fd);

    /**
     * 检查是否有数据
     * @return
     */
    long int check_read_count(int timeout_ms = 1);
};


#endif //TOOLS_POLL_TOOL_H
