//
// Created by caesar on 2019/12/30.
//

#ifndef KPROXYCPP_HTTP_H
#define KPROXYCPP_HTTP_H

#include <string>
#include <map>
#include <vector>
#include <CJsonObject.h>
#include <mutex>

#ifdef ENABLE_OPENSSL

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#endif

#ifdef ENABLE_CURL
#include <curl/curl.h>
#endif

class HTTP {
private:
    static const char *TAG;
#ifdef ENABLE_OPENSSL
    static SSL_CTX *ctx;
    static bool is_ssl_inited;
    static std::mutex init_ssl_mutex;
#endif
    static bool is_curl_global_inited;
    static std::mutex init_curl_global_init_mutex;
    static std::string url_regex;
public:
    typedef void (*callback)(HTTP *, int, std::vector<unsigned char> data, void *argv);

#ifdef ENABLE_OPENSSL

    static void init_ssl();

#endif

public:
#ifdef ENABLE_CURL
    CURL *curl = nullptr;
#endif
    std::string protocol;
    std::string host;
    std::string port;
    std::string url;
    std::map<std::string, std::string> GET;
    std::map<std::string, std::string> POST;
    std::map<std::string, std::string> HEADER;
    std::map<std::string, std::string> ResponseHeader;
    CJsonObject JSON;
    std::string method = "GET";
    std::string USERPWD;
    std::string http_version = "HTTP/1.1";
    std::string http_code = "200";
    std::string http_code_message = "";
    std::string ContentType = "text/html;charset=UTF-8";
    bool debug = false;

    int timeout = 5;
public:
    explicit HTTP(const std::string &full_url);
    ~HTTP();

    static void init_key_value(std::map<std::string, std::string> &data, std::string value);


    void push_GET(const std::map<std::string, std::string> &data);

    void push_POST(const std::map<std::string, std::string> &data);

    void push_HEADER(const std::map<std::string, std::string> &data);

    void push_JSON(const CJsonObject &data);

    void send(callback cb, void *argv = nullptr);

    int send(std::vector<unsigned char> &data);

    static size_t req_reply(void *ptr, size_t size, size_t nmemb, void *stream);

    static size_t req_header_reply(void *ptr, size_t size, size_t nmemb, void *stream);

    unsigned long int init_header(const char *data, unsigned long int size);
};


#endif //KPROXYCPP_HTTP_H
