//
// Created by caesar on 2019/12/1.
//

#ifndef KPROXYCPP_KCLIENT_H
#define KPROXYCPP_KCLIENT_H


#include <logger.h>
#include <vector>
#include <map>
#include <HttpResponseCode.h>
#include <socket.h>

class kHttpd;

class kHttpdClient {
private:
public:
private:
    static const char *TAG;
    static logger *_logger;
public:
    HttpResponseCode::ResponseCode response_code = HttpResponseCode::ResponseCode::OK;
    std::string method;
    std::string url_path;
    std::string http_version;
    std::string ContentType = "text/html;charset=UTF-8";
    std::vector<unsigned char> ResponseContent;
    std::map<std::string, std::string> header;
    std::map<std::string, std::string> response_header;
    std::vector<unsigned char> body_data;

    kHttpdClient(kHttpd *parent, int fd);

    kHttpdClient(kHttpd *parent, int fd, const std::map<std::string, std::string> &header,
                 std::vector<unsigned char> data, unsigned long int split_index, bool is_split_n,
                 std::string method,
                 std::string url_path,
                 std::string http_version,
                 kekxv::socket *_socket);

    ~kHttpdClient();

    int run();

private:
    kHttpd *parent = nullptr;
    bool need_socket = true;
    int fd = 0;
    kekxv::socket *_socket = nullptr;

    void init(kHttpd *_parent, int _fd);

    void init_header(const char *data, unsigned long int size, bool is_split_n);

    void send_header();

    void send_body();

private:
    static std::string get_localtime(time_t now = 0);


    friend class kHttpd;
};


#endif //KPROXYCPP_KCLIENT_H
