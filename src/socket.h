//
// Created by caesar on 2019/12/7.
//

#ifndef KPROXYCPP_SOCKET_H
#define KPROXYCPP_SOCKET_H

#ifdef WIN32
#ifdef _Tools_HEADER_
#define DLL_socket_Export  __declspec(dllexport)
#else
#define DLL_socket_Export  __declspec(dllimport)
#endif
#else
#define DLL_socket_Export
#endif

#include <vector>
#include <logger.h>
#ifdef WIN32
typedef int socklen_t;
#else
#include <poll.h>
#endif
namespace kekxv {

    class DLL_socket_Export socket {
    public:
        enum ErrorCode {
            Error_Cannot_Get_Socket = -99,
            Error_Cannot_Bind,
        };
    private:
        static const char *TAG;
        static logger *_logger;
        static bool was_sigaction;
    public:
        static void segv_error_handle(int v);

        /**
         * 打开并监听指定端口
         * @param port 端口号
         * @param ip 需要监听的地址
         * @param listen_count 监听数量
         * @return 操作句柄
         */
        static int listen(short port, const char *ip = "0.0.0.0", int listen_count = 512);

        /**
         * 同 accept
         * @param fd
         * @param remote_addr
         * @param sin_size
         * @return
         */
        static int accept(int fd, struct sockaddr *__restrict remote_addr, socklen_t *__restrict sin_size);


    private:
        int fd = 0;
        struct pollfd client{};
    public:
        explicit socket(int fd);

        /**
         * 发送数据
         * @param data 待发送的数据
         * @param offset 发送数据起始
         * @param len 发送数据长度
         * @return 同 write
         */
        long int send(std::vector<unsigned char> data, long int offset = 0, long int len = -1, int flags = 0);

        /**
         * 发送数据
         * @param data 待发送的数据
         * @param offset 发送数据起始
         * @param len 发送数据长度
         * @return 同 write
         */
        long int send(unsigned char *data, long int offset = 0, long int len = -1, int flags = 0);

        /**
         * 发送字符串
         * @param data 需要发送的字符串
         * @return
         */
        long int send(const std::string &data);

        /**
         * 等待发送完毕
         */
        void wait_send_finish();

        /**
         * 接收数据
         * @param data 接收数据存放位置
         * @return
         */
        long int read(std::vector<unsigned char> &data, int flags = 0);

        /**
         * 检查是否有数据
         * @return
         */
        long int check_read_count(int timeout_ms = 1);

        long int check_can_send(int timeout_ms = 1);

    };
}


#endif //KPROXYCPP_SOCKET_H
