//
// Created by caesar on 2019/12/7.
//

#include <netinet/in.h>
#include <arpa/inet.h>
#include "socket.h"
#include<stdio.h>
#include<signal.h>
#include<unistd.h>

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
        struct sigaction siga{}, old{};
        siga.sa_handler = segv_error_handle;
        siga.sa_flags = 0;
        memset(&siga.sa_mask, 0, sizeof(sigset_t));
        sigaction(SIGPIPE, &siga, &old);
        was_sigaction = true;
    }

    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        _logger->e(TAG, __LINE__, "cannot set sockopt SO_REUSEADDR");
    }


    if (fd < 0) {
        _logger->e(TAG, __LINE__, "cannot get socket");
        return Error_Cannot_Get_Socket;
    }

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

ssize_t kekxv::socket::send(std::vector<unsigned char> data, ssize_t offset, ssize_t len, int flags) {
    if (len < 0) {
        len = data.size() - offset;
    }
    return send(data.data(), offset, len, flags);
}

ssize_t kekxv::socket::send(unsigned char *data, ssize_t offset, ssize_t len, int flags) {
    if (check_can_send() < 0)return 0;
    return ::send(fd, &data[offset], len, flags);
}

ssize_t kekxv::socket::send(const std::string &data) {
    return send((unsigned char *) data.data(), 0, data.size());
}

void kekxv::socket::wait_send_finish() {
    // TODO 未完成
}

ssize_t kekxv::socket::read(std::vector<unsigned char> &data, int flags) {
    unsigned char buf[512];
    do {
        int ret = recv(fd, buf, 512, flags);
        if (ret == 0)return 0;
        data.insert(data.end(), &buf[0], &buf[ret]);
        if (ret != 512) {
            break;
        }
    } while (true);
    return data.size();
}

ssize_t kekxv::socket::check_read_count(int timeout) {
//    client.events = POLLIN | POLLPRI | POLLRDNORM;
    client.events = POLLIN;
    int poll_ret = poll(&client, 1, timeout);
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

ssize_t kekxv::socket::check_can_send(int timeout_ms) {
    client.events = POLLOUT | POLLWRBAND;
    int poll_ret = poll(&client, 1, timeout_ms);
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
