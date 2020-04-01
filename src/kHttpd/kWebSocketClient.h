//
// Created by caesar on 2019/12/11.
//

#ifndef KPROXYCPP_KWEBSOCKETCLIENT_H
#define KPROXYCPP_KWEBSOCKETCLIENT_H

#include <logger.h>
#include <vector>
#include <map>
#include <HttpResponseCode.h>
#include <socket.h>


class kHttpd;

class kWebSocketClient {
private:
public:
private:
    static const char *TAG;
    static logger *_logger;
public:
    HttpResponseCode::ResponseCode response_code = HttpResponseCode::ResponseCode::SwitchingProtocols;
    std::string method;
    std::string url_path;
    std::string http_version;
    std::string SecWebSocketAccept = "";
    std::string SecWebSocketKey = "";
    std::map<std::string, std::string> header;
    std::map<std::string, std::string> response_header;

    kWebSocketClient(kHttpd *parent, int fd);

    ~kWebSocketClient();

    int run();

    /**
     * 发送数据
     * @param data 数据内容
     * @param type 数据类型：0 连续的frame  1 文本frame  2 二进制frame  3-7 预留  8 连接关闭  9 ping  A pong  B-F 预留
     * @return
     */
    int send(const std::vector<unsigned char> &data, unsigned char type);
    int send(const std::string &data);
    inline int get_fd(){return fd;};

private:
    kHttpd *parent = nullptr;
    int fd = 0;
    kekxv::socket *_socket = nullptr;
    std::mutex m_send_lock;


    void init_header(const char *data, unsigned long int size, bool is_split_n);

    void send_header();

    int _run();

private:
    static std::string get_localtime(time_t now = 0);

    int parse(std::vector<unsigned char> &data, unsigned char &FIN, unsigned long long &next);

    static int build(std::vector<unsigned char> &send_data,const std::vector<unsigned char> &data, unsigned char type,
                     bool $need_masks = false);


    friend class kHttpd;
};


#endif //KPROXYCPP_KWEBSOCKETCLIENT_H
