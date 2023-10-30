#ifdef WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <cstdio>

#else

#include <netinet/in.h>
#include <arpa/inet.h>
#include<unistd.h>

#endif

#include <iostream>
#include <kHttpd.h>
#include <kWebSocketClient.h>
#include <base64.h>
#include <CJsonObject.hpp>
#include <kHttpdClient.h>
#include <getopt.h>
#include <string>

using namespace std;

void show_help() {
    printf(
            "help (http://kekxv.com)\n\n"
            "\t -d\t后台运行 \n"
            "\t -w\t开启 WebSocket \n"
            "\t -l\t设置监听地址 默认 0.0.0.0\n"
            "\t -p\t设置端口号 默认8080\n"
            "\t -P\t开启PHP，并设置 PHP 端口号\n"
            "\t -k\tssl 秘钥\n"
            "\t -K\tssl 公钥\n"
            "\t -n\t线程数量，默认 20，WebSocket 模式建议设置大一点\n"
            "\t -L\t静态文件路径 \n"
            "\t -h\t帮助信息 \n"
            "\t -?\t帮助信息 \n"
    );
}

int main(int argc, char **argv) {
    bool isWebSocket = false;
    bool httpd_option_daemon = false;
    string ip = "0.0.0.0";
    string php_ip;
    unsigned short port = 8080;
    unsigned short php_port = 9000;
    string web_root;
    unsigned int thread_num = 20;
    std::string certificate;
    std::string private_key;

    int opt;
    const char *string = "wdp:P:n:L:l:k:K:h?";
    while ((opt = getopt(argc, argv, string)) != -1) {
        switch (opt) {
            case 'w':
                isWebSocket = true;
                break;
            case 'd':
                httpd_option_daemon = true;
                break;
            case 'p':
                port = (int) strtol((const char *) optarg, nullptr, 10);
                break;
            case 'P':
                php_port = (int) strtol((const char *) optarg, nullptr, 10);
                php_ip = "127.0.0.1";
                break;
            case 'n':
                thread_num = (int) strtol((const char *) optarg, nullptr, 10);
                if (thread_num > 200)thread_num = 200;
                break;
            case 'L':
                web_root = optarg;
                break;
            case 'k':
                private_key = optarg;
                break;
            case 'K':
                certificate = optarg;
                break;
            case 'l':
                ip = optarg;
                break;
            default:
                show_help();
                exit(EXIT_SUCCESS);
        }
    }
#ifdef WIN32
#else
    //判断是否设置了-d，以daemon运行
    if (httpd_option_daemon) {
        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            //生成子进程成功，退出父进程
            exit(EXIT_SUCCESS);
        }
    }
#endif

    kHttpd::Init();


    kHttpd kProxy(web_root.c_str(), thread_num);
    kProxy.set_ssl_key(certificate, private_key);
    kProxy.isWebSocket = isWebSocket;
    if (!php_ip.empty() && php_port > 0) {
        kProxy.init_php(php_ip.c_str(), php_port);
    }

    kProxy.set_gencb([](void *kClient, const std::vector<unsigned char> &data, const std::string &url_path,
                        const std::string &method,
                        const std::string &host, int type, void *arg) -> int {
        return -1;
        // std::cout << "回调调用" << std::endl;
        // return ((kWebSocketClient *) kClient)->send(data, type) >= 0;
    });

    kProxy.set_cb([](void *kClient, const std::vector<unsigned char> &data, const std::string &url_path,
                     const std::string &method, int type, void *arg) -> int {
        //        std::cout << "回调调用:" << url_path << " " << method << " " << ((kWebSocketClient *) kClient)->header["host"]
        //                  << std::endl;

        if (type == 1) {
            /*
            return ((kWebSocketClient *) kClient)->send(
                    "回调调用:" + url_path + " " + method + " " + ((kWebSocketClient *) kClient)->header["host"]) >= 0 ? 0
                                                                                                                   : -1;
            */
            return ((kWebSocketClient *) kClient)->send(data, type) >= 0 ? 0 : -1;
        } else if (type == 8) {
            int _fd = ((kWebSocketClient *) kClient)->get_fd();
            // 关闭连接，可用于释放
            return 0;
        } else {
            return -1;
        }
        // return ((kWebSocketClient *) kClient)->send(data, type) >= 0;
    }, "/ws");


    kProxy.listen(20, port, ip.c_str());
#ifdef WIN32
    WSACleanup();
#endif
    return 0;
}