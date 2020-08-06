//
// Created by caesar kekxv on 2020/3/8.
//
#include <logger.h>

#ifdef WIN32
#include<winsock2.h>
#pragma   comment(lib,"WS2_32.lib")
#include<Ws2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#else

#include <netdb.h>
#include <arpa/inet.h>

#endif
using namespace std;

int main(int argc, char *argv[]) {
    logger::instance()->init_default();
#ifdef WIN32
    WSADATA wd;
    if(WSAStartup(MAKEWORD(2,2),&wd)  != 0) //WSAStartup
    {
        logger::instance()->e(TAG, __LINE__, "WSAStartup error！");
        return 1;
    }
#endif
    struct hostent *h;
    string hostname = "kekxv.com";
    if (argc == 2) {
        hostname = argv[1];
    }
    if ((h = gethostbyname(hostname.c_str())) == nullptr) {
        logger::instance()->e(TAG, __LINE__, "不能得到IP");
        exit(1);
    }
    logger::instance()->i(TAG, __LINE__, "HostName :%s", h->h_name);
    int i = 0;
    while (h->h_addr_list[i]) {
        logger::instance()->i(TAG, __LINE__, "[%02d] IP Address :%s", i + 1,
                              inet_ntoa(*((struct in_addr *) h->h_addr_list[i])));
        i++;
    }
    return EXIT_SUCCESS;
}