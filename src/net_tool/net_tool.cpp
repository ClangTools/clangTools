//
// Created by caesar kekxv on 2020/3/30.
//

#include "net_tool.h"
#include <cstdio>

#ifdef WIN32

#include <iostream>
#include <winsock2.h>

#else

#include <sys/socket.h>
#include <sys/ioctl.h>

#ifdef __linux__

#include <linux/netdevice.h>

#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#endif

#include <logger.h>

#ifdef __FILENAME__
const char *net_tool::TAG = __FILENAME__;
#else
const char *net_tool::TAG = "net_tool";
#endif
int net_tool::GetIP(std::vector<std::string> &ips, bool hasIpv6) {
#ifdef __unix__
    int domains[] = {AF_INET, AF_INET6};
    int i;
    for (i = 0; i < sizeof(domains) / sizeof(domains[0]); i++) {
        int domain = domains[i];
        int s;
        struct ifconf ifconf{};
        struct ifreq ifr[50];
        unsigned int ifs;
        int j;
        s = socket(domain, SOCK_STREAM, 0);
        if (s < 0) {
            logger::instance()->e(TAG, __LINE__, "socket : %s", strerror(errno));
            return 0;
        }
        ifconf.ifc_buf = (char *) ifr;
        ifconf.ifc_len = sizeof ifr;
        if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
            logger::instance()->e(TAG, __LINE__, "ioctl : %s", strerror(errno));
            return 0;
        }
        ifs = ifconf.ifc_len / sizeof(ifr[0]);
        // logger::instance()->d(TAG, __LINE__, "interfaces = %d ", ifs);
        for (j = 0; j < ifs; j++) {
            char ip[INET_ADDRSTRLEN];
            auto *s_in = (struct sockaddr_in *) &ifr[j].ifr_addr;
            if (!inet_ntop(domain, &s_in->sin_addr, ip, sizeof(ip))) {
                // logger::instance()->e(TAG, __LINE__, "inet_ntop : %s", strerror(errno));
                continue;
            }
            // logger::instance()->d(TAG, __LINE__, "%s : %s", ifr[j].ifr_name, ip);
            ips.emplace_back(ip);
        }
        close(s);
        if (!hasIpv6)break;
    }
#endif

#ifdef WIN32
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
        return -1;
    char local[255] = {0};
    gethostname(local, sizeof(local));
    hostent *ph = gethostbyname(local);
    if (ph == NULL)
        return -1;

    for (int i = 0;; i++) {
        std::string localIP = inet_ntoa(*(IN_ADDR *) ph->h_addr_list[i]);
        ips.push_back(localIP);
        if (ph->h_addr_list[i] + ph->h_length >= ph->h_name)
            break;
    }
    //in_addr addr;
    //memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr)); // 这里仅获取第一个ip
    //localIP.assign(inet_ntoa(addr));
    WSACleanup();
#endif
    return 0;
}
