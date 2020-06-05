//
// Created by caesar kekxv on 2020/3/30.
//

#ifndef TOOLS_NET_TOOL_H
#define TOOLS_NET_TOOL_H

#include <vector>
#include <string>

class net_tool {
public:
    static const char *TAG;

    static int GetIP(std::vector<std::string> &ip, bool hasIpv6 = false);
};


#endif //TOOLS_NET_TOOL_H
