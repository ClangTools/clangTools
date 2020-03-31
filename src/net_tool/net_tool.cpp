//
// Created by caesar kekxv on 2020/3/30.
//

#include "net_tool.h"
#include <cstdio>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <logger.h>

int net_tool::GetIP(std::vector<std::string> &ips) {
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
            logger::instance()->e(__FILENAME__, __LINE__, "socket : %s", strerror(errno));
            return 0;
        }
        ifconf.ifc_buf = (char *) ifr;
        ifconf.ifc_len = sizeof ifr;
        if (ioctl(s, SIOCGIFCONF, &ifconf) == -1) {
            logger::instance()->e(__FILENAME__, __LINE__, "ioctl : %s", strerror(errno));
            return 0;
        }
        ifs = ifconf.ifc_len / sizeof(ifr[0]);
        logger::instance()->d(__FILENAME__, __LINE__, "interfaces = %d ", ifs);
        for (j = 0; j < ifs; j++) {
            char ip[INET_ADDRSTRLEN];
            auto *s_in = (struct sockaddr_in *) &ifr[j].ifr_addr;
            if (!inet_ntop(domain, &s_in->sin_addr, ip, sizeof(ip))) {
                logger::instance()->e(__FILENAME__, __LINE__, "inet_ntop : %s", strerror(errno));
                continue;
            }
            logger::instance()->d(__FILENAME__, __LINE__, "%s : %s", ifr[j].ifr_name, ip);
            ips.emplace_back(ip);
        }
        close(s);
    }
    return 0;
}
