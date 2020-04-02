//
// Created by caesar on 2019/11/26.
//

#ifdef WIN32

#include <WS2tcpip.h>
#include <WinSock2.h>
#include <mstcpip.h>
#include <cstdio>

#else

#include <netinet/in.h>
#include <arpa/inet.h>
#include<unistd.h>

#endif

#include "kHttpd.h"
#include <utility>
#include <vector>


#include <kHttpdClient.h>
#include <kWebSocketClient.h>
#include <socket.h>
#include <CGI/kCGI.h>

#ifdef ENABLE_OPENSSL

#include <ssl_common.h>

#endif

#include <net_tool.h>

using namespace std;
using namespace kekxv;

#ifdef __FILENAME__
const char *kHttpd::TAG = __FILENAME__;
#else
const char* kHttpd::TAG = "kHttpd";
#endif

logger *kHttpd::_logger = logger::instance();;

void kHttpd::Init() {
    _logger->min_level = logger::log_rank_DEBUG;
#ifdef _LOGGER_USE_THREAD_POOL_
    _logger->wait_show = false;
#endif
    _logger->console_show = true;

#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    (void) WSAStartup(wVersionRequested, &wsaData);
#endif

}

kHttpd::kHttpd(unsigned int max_thread) {
    this->threadPool = new thread_pool(max_thread);
}

kHttpd::kHttpd(const char *web_root_path, unsigned int max_thread) {
    this->threadPool = new thread_pool(max_thread);
    if (web_root_path != nullptr) {
#ifdef _WIN32
        const char *path = _fullpath(nullptr, web_root_path, 1024);
#else
        const char *path = realpath(web_root_path, nullptr);
#endif
        if (path != nullptr)
            this->web_root_path = path;
    }
}


kHttpd::~kHttpd() {
#ifdef ENABLE_OPENSSL
    std::unique_lock<mutex> lock(sslLock);
    use_ssl = false;
    if (sslCommon != nullptr) {
        delete sslCommon;
        sslCommon = nullptr;
    }
    sslLock.unlock();
#endif
    delete this->threadPool;
    this->threadPool = nullptr;
}

/**
 * 开启并设置 ssl 证书
 * @param _certificate
 * @param _private_key
 * @return 0 开启成功
 */
void kHttpd::set_ssl_key(const std::string &_certificate, const std::string &_private_key) {
    this->certificate = _certificate;
    this->private_key = _private_key;
#ifdef ENABLE_OPENSSL
    std::unique_lock<mutex> lock(sslLock);
    sslCommon = new ssl_common(_certificate.c_str(), _private_key.c_str());
    if (sslCommon->isReady()) {
        use_ssl = true;
    } else {
        use_ssl = false;
        if (sslCommon != nullptr) {
            delete sslCommon;
            sslCommon = nullptr;
        }
    }
    sslLock.unlock();
#endif
}

int kHttpd::listen(int listen_count, unsigned short port, const char *ip) {
    this->fd = kekxv::socket::listen(port, ip, listen_count);
    if (fd <= 0) {
        _logger->e(TAG, __LINE__, "cannot listen");
        return fd;
    }

    _logger->d(__FILENAME__, __LINE__, " root : %s", web_root_path.c_str());
    if (ip == std::string("0.0.0.0")) {
        std::vector<std::string> ips;
        net_tool::GetIP(ips);
        for (auto _ip:ips)
            _logger->i(__FILENAME__, __LINE__, " http://%s:%d", _ip.c_str(), port);
    }
    _logger->i(__FILENAME__, __LINE__, " http://%s:%d", ip, port);
    //    std::thread _poll_check_run(poll_check_run, this);

    do {

        long int list_len = socket_fd_list.size();
        auto *poll_fd = new struct pollfd[list_len + 1];
        for (long int i = 0; i < list_len; i++) {
            poll_fd[i].fd = socket_fd_list[i];
#ifdef WIN32
            poll_fd[i].events = POLLWRNORM | POLLOUT | POLLIN;
            // poll_fd[i].events &= ~(POLLPRI | POLLWRBAND);
#else
            poll_fd[i].events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND | \
                POLLERR | POLLHUP | POLLNVAL;
#endif
            poll_fd[i].revents = 0;
        }
        poll_fd[list_len].fd = fd;
#ifdef WIN32
        poll_fd[list_len].events = POLLWRNORM | POLLOUT | POLLIN;
#else
        poll_fd[list_len].events = POLLIN | POLLPRI | POLLRDNORM | POLLRDBAND | \
            POLLERR | POLLHUP | POLLNVAL;;
#endif
        poll_fd[list_len].revents = 0;
#ifdef WIN32
        int num_ready = WSAPoll(poll_fd, list_len + 1, -1);
#else
        int num_ready = poll(poll_fd, list_len + 1, -1);
#endif
        if ((num_ready == -1) &&
            #ifdef WIN32
            (WSAGetLastError() == WSAEINTR)
#else
            (errno == EINTR)
#endif
                ) {
            // lock.unlock();
            delete[] poll_fd;
            continue;        //被信号中断，继续等待
        } else if (num_ready == 0) {
            // lock.unlock();
            delete[] poll_fd;
#ifdef WIN32
            Sleep(1);
#else
            usleep(1);
#endif
            continue;
        } else if (0 > num_ready) {
            for (long int i = list_len - 1; i >= 0; i--) {
#ifdef WIN32
                // shutdown(socket_fd_list[i], SD_BOTH);
                closesocket(socket_fd_list[i]);
#else
                shutdown(socket_fd_list[i], SHUT_RDWR);
                close(socket_fd_list[i]);
#endif
            }
            socket_fd_list.clear();
            delete[] poll_fd;
            continue;
        }
        if (poll_fd[list_len].revents > 0) {
            int new_fd = kekxv::socket::accept(fd, nullptr, nullptr);
            if (new_fd < 0) {
                _logger->e(TAG, __LINE__, "Accept error in on_accept()");
#ifdef WIN32
                _logger->e(TAG, __LINE__, "Accept error WSAGetLastError : %d", WSAGetLastError());
                WSADATA wsa;
                WSAStartup(MAKEWORD(2, 2), &wsa);
                delete[] poll_fd;
                return -1;
#endif
            } else {
                add_poll_check(new_fd);
            }
            num_ready--;
        }
        for (long int i = list_len - 1; i >= 0 && num_ready > 0; i--) {
            if (poll_fd[i].revents & POLLIN
                || poll_fd[i].revents & POLLPRI
                || poll_fd[i].revents & POLLRDNORM
                || poll_fd[i].revents & POLLRDBAND
                || poll_fd[i].revents & POLLWRNORM
                || poll_fd[i].revents & POLLOUT
                    ) {
                threadPool->commit([this](int new_fd) -> void {
                    // _logger->d(TAG, __LINE__, "======== client thread run ========");
                    int _fd = -1;
                    if (isWebSocket) {
                        _fd = kWebSocketClient(this, new_fd).run();
                    } else {
                        _fd = kHttpdClient(this, new_fd).run();
                    }
                    if (_fd > 0) {
#ifdef WIN32
                        // shutdown(_fd, SD_BOTH);
                        closesocket(_fd);
#else
                        shutdown(_fd, SHUT_RDWR);
                        close(_fd);
#endif
                        // self->add_poll_check(_fd);
                    }
                    // _logger->d(TAG, __LINE__, "======== client thread end ========");
                }, poll_fd[i].fd);
                this->socket_fd_list.erase(this->socket_fd_list.begin() + i);
                num_ready--;
            } else if (poll_fd[i].revents & POLLERR
                       || poll_fd[i].revents & POLLHUP
                       || poll_fd[i].revents & POLLNVAL
                    ) {
#ifdef WIN32
                // shutdown(poll_fd[i].fd, SD_BOTH);
                closesocket(poll_fd[i].fd);
#else
                shutdown(poll_fd[i].fd, SHUT_RDWR);
                close(poll_fd[i].fd);
#endif
                socket_fd_list.erase(socket_fd_list.begin() + i);
                num_ready--;
            }
        }
        delete[] poll_fd;
    } while (isRunning);

    stoped.store(true);
    //    _poll_check_run.join();
    _logger->i(TAG, __LINE__, "finished");
    return 0;
}

void kHttpd::add_poll_check(int new_fd) {
    // std::unique_lock<std::mutex> lock{this->m_lock};
    socket_fd_list.push_back(new_fd);
    // lock.unlock();
    // cv_task.notify_one();
}

int
kHttpd::check_host_path(class kHttpdClient *_kHttpdClient, const std::string &Host, const std::string &method,
                        const std::string &url_path) {
    if (url_cb_tasks.find(Host + "_" + method + "_" + url_path) != url_cb_tasks.end()) {
        return url_cb_tasks[Host + "_" + method + "_" + url_path](_kHttpdClient, _kHttpdClient->body_data, url_path,
                                                                  method, -1, nullptr);
    } else if (url_cb_tasks.find(Host + "_" + "_" + url_path) != url_cb_tasks.end()) {
        return url_cb_tasks[Host + "_" + "_" + url_path](_kHttpdClient, _kHttpdClient->body_data, url_path,
                                                         method, -1, nullptr);
    } else if (url_cb_tasks.find(method + "_" +
                                 url_path) != url_cb_tasks.end()) {
        return url_cb_tasks[method + "_" +
                            url_path](_kHttpdClient, _kHttpdClient->body_data, url_path,
                                      method, -1, nullptr);
    } else if (url_cb_tasks.find(url_path) != url_cb_tasks.end()) {
        return url_cb_tasks[url_path](_kHttpdClient, _kHttpdClient->body_data, url_path,
                                      method, -1, nullptr);
    } else if (gen_cb_task != nullptr) {
        return gen_cb_task(_kHttpdClient, _kHttpdClient->body_data, url_path,
                           method,
                           Host, -1, nullptr);
    }
    return -1;
}

int
kHttpd::check_host_path(class kWebSocketClient *_kWebSocketClient, int type, const std::vector<unsigned char> &data) {
    if (url_cb_tasks.find(_kWebSocketClient->header["host"] + "_" + _kWebSocketClient->method + "_" +
                          _kWebSocketClient->url_path) != url_cb_tasks.end()) {
        return url_cb_tasks[_kWebSocketClient->header["host"] + "_" + _kWebSocketClient->method + "_" +
                            _kWebSocketClient->url_path](_kWebSocketClient, data, _kWebSocketClient->url_path,
                                                         _kWebSocketClient->method, type, nullptr);
    } else if (url_cb_tasks.find(_kWebSocketClient->header["host"] + "_" + "_" +
                                 _kWebSocketClient->url_path) != url_cb_tasks.end()) {
        return url_cb_tasks[_kWebSocketClient->header["host"] + "_" + "_" +
                            _kWebSocketClient->url_path](_kWebSocketClient, data, _kWebSocketClient->url_path,
                                                         _kWebSocketClient->method, type, nullptr);
    } else if (url_cb_tasks.find(_kWebSocketClient->method + "_" +
                                 _kWebSocketClient->url_path) != url_cb_tasks.end()) {
        return url_cb_tasks[_kWebSocketClient->method + "_" +
                            _kWebSocketClient->url_path](_kWebSocketClient, data, _kWebSocketClient->url_path,
                                                         _kWebSocketClient->method, type, nullptr);
    } else if (url_cb_tasks.find(_kWebSocketClient->url_path) != url_cb_tasks.end()) {
        return url_cb_tasks[_kWebSocketClient->url_path](_kWebSocketClient, data, _kWebSocketClient->url_path,
                                                         _kWebSocketClient->method, type, nullptr);
    } else if (gen_cb_task != nullptr) {
        return gen_cb_task(_kWebSocketClient, data, _kWebSocketClient->url_path,
                           _kWebSocketClient->method,
                           _kWebSocketClient->header["host"], type, nullptr);
    }
    return _kWebSocketClient->send(data, type) >= 0;
}

void kHttpd::set_cb(
        const std::string &method,
        const std::string &url_path,
        url_cb task,
        const std::string &host) {
    set_cb(task, url_path, method, host);
}

void kHttpd::set_cb(
        const std::string &url_path,
        const std::string &method,
        const std::string &host,
        url_cb task) {
    set_cb(task, url_path, method, host);
}

void
kHttpd::set_cb(kHttpd::url_cb task, const std::string &url_path, const std::string &method, const std::string &host) {
    string key;
    if (!host.empty()) {
        key += host + "_";
        key += method + "_";
    }
    if (!method.empty()) {
        key += method + "_";
    }
    key += url_path;
    url_cb_tasks[key] = std::move(task);
}

void kHttpd::set_gencb(kHttpd::gen_cb task) {
    gen_cb_task = std::move(task);
}

void kHttpd::init_php(const char *SockPath) {
    if (SockPath != nullptr)PhpSockPath = SockPath;
}

void kHttpd::init_php(const char *ip, unsigned short port) {
    if (ip != nullptr) {
        PhpIp = ip;
        PhpPort = port;
    }
}

void kHttpd::RunPhpCGI(const string &filePath, clangTools::kCGI &kCgi,
                       kHttpdClient *httpdClient,
                       map<string, string> &header,
                       vector<unsigned char> &data) {
    kCgi.sendStartRequestRecord();
    kCgi.sendParams("SCRIPT_FILENAME", filePath.c_str());
    kCgi.sendParams("REQUEST_METHOD", httpdClient->method.c_str());
    kCgi.sendParams("REMOTE_HOST", httpdClient->header["host"].c_str());
    kCgi.sendParams("SERVER_NAME", httpdClient->header["host"].c_str());
    kCgi.sendParams("SERVER_SOFTWARE", "kHttpd v 0.1");
    kCgi.sendParams("HTTP_COOKIE", httpdClient->header["cookie"].c_str());
    kCgi.sendEndRequestRecord();
    //        kCgiData kData;
    kCgi.ReadFromPhp(header, data);
    //        kData.ToVector(data);
}
