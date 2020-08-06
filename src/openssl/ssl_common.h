/*********************************
 * Created by caesar on 2019/12/7.
 * openssl 封装
 *
 * 本文件来自 https://github.com/kekxv/
 *
 * 参考自 https://github.com/darrenjs/openssl_examples
 *********************************/
#ifndef __SSL_COMMON_HEADER_
#define __SSL_COMMON_HEADER_

#ifdef ENABLE_OPENSSL
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#endif

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <logger.h>

namespace kekxv {
    class socket;
};

class ssl_common {
public:
    enum SSL_status {
        SSL_STATUS_OK, SSL_STATUS_WANT_IO, SSL_STATUS_FAIL
    };
    enum SSL_mode {
        SSL_MODE_SERVER, SSL_MODE_CLIENT
    };
private:
    static std::mutex is_init_ssl_lock;
    static bool is_init_ssl;
#ifdef ENABLE_OPENSSL
    SSL_CTX *ctx = nullptr;
#endif
    std::mutex ssl_lock;
public:
    static const char *TAG;
    const int DEFAULT_BUF_SIZE = 64;
    SSL_mode ssl_mode = SSL_MODE_SERVER;

    class ssl_common_sub;

    ssl_common(const char *cert_file, const char *key_file);

    ~ssl_common();

    bool isReady();

    bool init_sub(ssl_common_sub *&sub);

    void free_sub(ssl_common_sub *&sub);

public:
    class ssl_common_sub {
    public:
        int fd = -1;
    private:
        ssl_common *parent = nullptr;
#ifdef ENABLE_OPENSSL
        SSL *ssl = nullptr;
        BIO *rBIO = nullptr; /* SSL reads from, we write to. */
        BIO *wBIO = nullptr; /* SSL writes to, we read from. */
#endif
    public:
        std::vector<unsigned char> write_buf;
        std::vector<unsigned char> encrypt_buf;
    public:
        explicit ssl_common_sub(ssl_common *parent);

        ~ssl_common_sub();

        enum SSL_status get_ssl_status(int n);

        void send_unencrypted_bytes(unsigned char *buf, size_t len);

        void queue_encrypted_bytes(unsigned char *buf, size_t len);

        enum SSL_status do_ssl_handshake();

        bool SSL_init_finished();

        int on_read_cb(std::vector<unsigned char> &data, unsigned char *src, size_t len);

        int do_encrypt(std::vector<unsigned char> &encrypt_data, std::vector<unsigned char> data);

    public:
        friend class kekxv::socket;

    };

    friend class ssl_common_sub;
};


#endif // __SSL_COMMON_HEADER_