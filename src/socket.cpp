//
// Created by caesar on 2019/12/7.
//


#include "socket.h"
#include<cstdio>
#include<csignal>

#ifdef WIN32
#else
#include <fcntl.h>
#include <netdb.h>
#endif

using namespace std;
using namespace kekxv;

#ifdef __FILENAME__
const char *socket::TAG = __FILENAME__;
#else
const char *socket::TAG = "socket";
#endif
logger *socket::_logger = logger::instance();
bool socket::was_sigaction = false;

void socket::segv_error_handle(int v) {
    // _logger->e(TAG, __LINE__, "signal error handle:%d",v);
}

int socket::listen(short port, const char *ip, int listen_count) {
    if (!was_sigaction) {
#ifdef WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#else
        struct sigaction siga{}, old{};
        siga.sa_handler = segv_error_handle;
        siga.sa_flags = 0;
        memset(&siga.sa_mask, 0, sizeof(sigset_t));
        sigaction(SIGPIPE, &siga, &old);
#endif
        was_sigaction = true;
    }

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
#ifdef WIN32
    if (fd == INVALID_SOCKET)
        return Error_Cannot_Get_Socket;
    ULONG uNonBlockingMode = 1;
    if (SOCKET_ERROR == ioctlsocket(fd, FIONBIO, &uNonBlockingMode)) {
        _logger->e(TAG, __LINE__, "cannot set sockopt SO_REUSEADDR");
    }
#else
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        _logger->e(TAG, __LINE__, "cannot set sockopt SO_REUSEADDR");
    }
#endif

    if (fd < 0) {
        _logger->e(TAG, __LINE__, "cannot get socket");
        return Error_Cannot_Get_Socket;
    }

#ifdef WIN32
    SOCKADDR_STORAGE addr = {0};
    addr.ss_family = AF_INET;
    INETADDR_SETANY((SOCKADDR *) &addr);
    SS_PORT((SOCKADDR *) &addr) = htons(port);
    if (SOCKET_ERROR == ::bind(fd, (SOCKADDR *) &addr, sizeof(addr))) {
        _logger->e(TAG, __LINE__, "bind error %d", SOCKET_ERROR);
        return Error_Cannot_Bind;
    }
    if (SOCKET_ERROR == ::listen(fd, listen_count)) {
        _logger->e(TAG, __LINE__, "listen error %d", SOCKET_ERROR);
        return Error_Cannot_Bind;
    }
#else
    struct sockaddr_in serv{};

    memset(&serv, 0, sizeof(serv));
    // serv.sin_addr.s_addr = htonl(INADDR_ANY);
    serv.sin_family = AF_INET;
    inet_aton(ip, &serv.sin_addr);
    serv.sin_port = htons(port);

    int bind_result = ::bind(fd, (struct sockaddr *) &serv, sizeof(struct sockaddr));
    if (bind_result < 0) {
        _logger->e(TAG, __LINE__, "bind error %d", bind_result);
        return Error_Cannot_Bind;
    }
    ::listen(fd, listen_count);
#endif

    _logger->d(__FILENAME__, __LINE__, " tcp://%s:%d", ip, port);

    return fd;
}

int kekxv::socket::accept(int fd, struct sockaddr *remote_addr, socklen_t *sin_size) {
    return ::accept(fd, remote_addr, sin_size);
}

kekxv::socket::socket(int fd) {
    this->fd = fd;
    client.fd = fd;
    client.events = POLLIN;
    client.revents = 0;
}

#ifdef ENABLE_OPENSSL

kekxv::socket::socket(int fd, ssl_common *sslCommon) {
    this->fd = fd;
    client.fd = fd;
    client.events = POLLIN;
    client.revents = 0;
    this->sslCommon = sslCommon;
    if (sslCommon != nullptr && sslCommon->isReady()) {
        if (sslCommonSub != nullptr)
            sslCommon->free_sub(sslCommonSub);
        sslCommon->init_sub(sslCommonSub);
        if (!do_ssl_handshake()) {
            sslCommon->free_sub(sslCommonSub);
        }
    }
}

bool socket::do_ssl_handshake() {
    vector<unsigned char> data;
    while (!sslCommonSub->SSL_init_finished()) {
        vector<unsigned char> buf;
        if (this->check_read_count(1000, false) <= 0) {
            return false;
        }
        this->read(buf, 0, false);
        if (buf.empty())
            return false;
        data.insert(data.end(), buf.begin(), buf.end());

        int len = sslCommon->DEFAULT_BUF_SIZE;
        while (!sslCommonSub->SSL_init_finished() && !buf.empty()) {
            int n = BIO_write(sslCommonSub->rBIO, &buf.data()[0],
                              (buf.size() < len) ? (buf.size()) : len);
            if (n <= 0)
                return false; /* assume bio write failure is unrecoverable */
            buf.erase(buf.begin(), buf.begin() + n);

            auto ret = sslCommonSub->do_ssl_handshake();
            if (ret == ssl_common::SSL_status::SSL_STATUS_FAIL) {
                ssl_data.insert(ssl_data.end(), data.begin(), data.end());
                return false;
            }
            if (sslCommonSub->SSL_init_finished()) {
                if (!buf.empty())
                    ssl_data.insert(ssl_data.end(), buf.begin(), buf.end());
                unsigned char _buf[1024];
                n = BIO_read(sslCommonSub->wBIO, _buf, sizeof(_buf));
                if (n > 0) {
                    this->send(_buf, 0, n, 0, false);
                    // ssl_data.insert(ssl_data.end(), &_buf[0], &_buf[n]);
                }
                break;
            }
            if (!sslCommonSub->write_buf.empty()) {
                this->send(sslCommonSub->write_buf, 0, -1, 0, false);
                sslCommonSub->write_buf.clear();
            }
        }
    }

    return sslCommonSub->SSL_init_finished();
}

#endif

long int kekxv::socket::send(std::vector<unsigned char> data, long int offset, long int len, int flags, bool is_ssl) {
    if (len < 0) {
        len = data.size() - offset;
    }
    return send(data.data(), offset, len, flags, is_ssl);
}

long int kekxv::socket::send(unsigned char *data, long int offset, long int len, int flags, bool is_ssl) {
    if (check_can_send() < 0)return 0;
#ifdef ENABLE_OPENSSL
    if (is_ssl && sslCommonSub != nullptr) {
        vector<unsigned char> buffer;
        sslCommonSub->do_encrypt(buffer, vector<unsigned char>(&data[offset], &data[offset + len]));
        if (!buffer.empty()) {

#ifdef WIN32
            return ::send(fd, (const char *) buffer.data(), buffer.size(), flags);
#else
            return ::send(fd, buffer.data(), buffer.size(), flags);
#endif
        }
    }
#endif
#ifdef WIN32
    return ::send(fd, (const char *) &data[offset], len, flags);
#else
    return ::send(fd, &data[offset], len, flags);
#endif
}

long int kekxv::socket::send(const std::string &data, bool is_ssl) {
    return send((unsigned char *) data.data(), 0, data.size(), 0, is_ssl);
}

void kekxv::socket::wait_send_finish() {
    // TODO 未完成
}


long int kekxv::socket::read(std::vector<unsigned char> &data, int flags, bool is_ssl) {
#ifdef ENABLE_OPENSSL
    if (is_ssl && !ssl_data.empty()) {
        data.insert(data.end(), ssl_data.begin(), ssl_data.end());
        ssl_data.clear();
        return data.size();
    }
#endif
    unsigned char buf[512] = {};
    do {
        int ret = 0;

#ifdef WIN32
        ret = ::recv(fd, (char *) buf, 512, flags);
#else
        ret = ::recv(fd, buf, 512, flags);
#endif
        if (ret <= 0)
            return 0;
        data.insert(data.end(), &buf[0], &buf[ret]);
        if (ret != 512) {
            break;
        }
    } while (true);
#ifdef ENABLE_OPENSSL
    if (is_ssl && sslCommonSub != nullptr) {
        vector<unsigned char> buffer;
        int n = BIO_write(sslCommonSub->rBIO, data.data(), data.size());
        if (n > 0) {
            do {
                n = SSL_read(sslCommonSub->ssl, buf, sizeof(buf));
                if (n > 0)
                    buffer.insert(buffer.end(), &buf[0], &buf[n]);
            } while (n > 0);
            if (!buffer.empty()) {
                data.clear();
                data.insert(data.end(), buffer.begin(), buffer.end());
            }

//            data.push_back(0);
//            _logger->d(TAG, __LINE__, "%s", data.data());
//            data.erase(data.end() - 1);
        }
    }
#endif
    return data.size();
}

long int kekxv::socket::check_read_count(int timeout, bool is_ssl) {
#ifdef ENABLE_OPENSSL
    if (is_ssl && !ssl_data.empty()) {
        return ssl_data.size();
    }
#endif
    client.revents = 0;
#ifdef WIN32
    client.events = POLLIN;
    int poll_ret = WSAPoll(&client, 1, timeout);
#else
    //    client.events = POLLIN | POLLPRI | POLLRDNORM;
    client.events = POLLIN;
    int poll_ret = poll(&client, 1, timeout);
#endif
    if (poll_ret < 0) {
        if (errno != EINTR) {
            _logger->e(TAG, __LINE__, "poll is not EINTR ;errno is %d ", errno);
            return poll_ret;
        }
        _logger->w(TAG, __LINE__, "poll is EINTR");
        return poll_ret;
    } else if (poll_ret == 0) {
        // _logger->d(TAG, __LINE__, "poll is time out");
        return poll_ret;
    } else if (poll_ret > 0) {
        // _logger->d(TAG, __LINE__, "%s POLLIN is %d", __FUNCTION__, client.fd);
        return poll_ret;
    }
    return 1;
}

long int kekxv::socket::check_can_send(int timeout_ms) {
    int poll_ret = 0;
#ifdef WIN32
    client.events = POLLOUT;
    poll_ret = WSAPoll(&client, 1, timeout_ms);
#else
    client.events = POLLOUT | POLLWRBAND;
    poll_ret = poll(&client, 1, timeout_ms);
#endif
    if (poll_ret < 0) {
        if (errno != EINTR) {
            _logger->e(TAG, __LINE__, "poll is not EINTR ;errno is %d ", errno);
            return poll_ret;
        }
        _logger->w(TAG, __LINE__, "poll is EINTR");
        return poll_ret;
    } else if (poll_ret == 0) {
        _logger->w(TAG, __LINE__, "poll is time out");
        return poll_ret;
    } else if (poll_ret > 0) {
        // _logger->d(TAG, __LINE__, "%s POLLIN,client.revents:is 0x%x", __FUNCTION__, client.revents);
        return poll_ret;
    }
    return 1;
}

kekxv::socket::~socket() {
#ifdef ENABLE_OPENSSL
    if (sslCommonSub != nullptr) {
        sslCommon->free_sub(sslCommonSub);
        sslCommon = nullptr;
    }
#endif
    if (need_close) {
        close(fd);
    }
}


int socket::OpenConnection(const char *hostname, const char *port) {
    struct hostent *_host = nullptr;
    if ((_host = gethostbyname(hostname)) == nullptr) {
        logger::instance()->e(TAG, __LINE__, "%s: %s", hostname, strerror(errno));
        return -1;
    }

    struct addrinfo hints = {0}, *addrs = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    const int status = getaddrinfo(hostname, port, &hints, &addrs);
    if (status != 0) {
        logger::instance()->e(TAG, __LINE__, "%s: %s", hostname, strerror(errno));
        return -1;
    }

    int sfd = 0, err = 0;
    for (struct addrinfo *addr = addrs; addr != nullptr; addr = addr->ai_next) {
        sfd = ::socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
        if (sfd == -1) {
            err = errno;
            continue;
        }

        if (connect(sfd, addr->ai_addr, addr->ai_addrlen) == 0) {
            break;
        }

        err = errno;
        sfd = -1;
        close(sfd);
    }

    freeaddrinfo(addrs);

    if (sfd == -1) {
        logger::instance()->e(TAG, __LINE__, "%s: %s", hostname, strerror(err));
        return -1;
    }

    int reuse = 1;
#ifdef WIN32
    ULONG uNonBlockingMode = 1;
    if (SOCKET_ERROR == ioctlsocket(sfd, FIONBIO, &uNonBlockingMode)) {
        _logger->e(TAG, __LINE__, "cannot set sockopt SO_REUSEADDR");
    }
#else
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        _logger->e(TAG, __LINE__, "cannot set sockopt SO_REUSEADDR");
    }
#endif
    return sfd;
}

bool kekxv::socket::isReady() {
    return fd > 0;
}

kekxv::socket::socket(const char *hostname, const char *port) {
    if (!was_sigaction) {
#ifdef WIN32
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
#else
        struct sigaction siga{}, old{};
        siga.sa_handler = segv_error_handle;
        siga.sa_flags = 0;
        memset(&siga.sa_mask, 0, sizeof(sigset_t));
        sigaction(SIGPIPE, &siga, &old);
#endif
        was_sigaction = true;
    }
    need_close = true;
    fd = OpenConnection(hostname, port);
    client.fd = fd;
}
