//
// Created by caesar on 2020/1/6.
//

#include "Pipe.h"
#include <logger.h>

#ifdef _WIN32
#else

#include <zconf.h>

#include <utility>
#include <sys/stat.h>
#include <fcntl.h>

#endif

#ifdef __FILENAME__
const char *Pipe::TAG = __FILENAME__;

#else
const char* Pipe::TAG = "Pipe";
#endif

Pipe::Pipe(const std::string &name) {
#ifdef _WIN32
    this->pipe_name = R"(\\.\Pipe\)" + name;
#else
    this->pipe_name = "/tmp/" + name;
#endif
}

Pipe::~Pipe() {
    Free();
}

int Pipe::Init(bool _is_parent) {
    this->is_parent = _is_parent;
#ifdef _WIN32
#else
    if (access((pipe_name + (_is_parent ? "_1" : "_2")).c_str(), F_OK))/*判断是否已经创建了有名管道，如果已经创建，则返回0否则返回非0的数*/
    {
        int ret = mkfifo((pipe_name + (_is_parent ? "_1" : "_2")).c_str(), 0777); /*创建有名管道,成功返回0,失败返回-1*/
        if (ret == -1) /*创建有名管道失败*/
        {
            logger::instance()->e(TAG, __LINE__, "mkfifo %d:%s", errno, strerror(errno));
            return -1;
        }
    }
    if (access((pipe_name + (_is_parent ? "_2" : "_1")).c_str(), F_OK))/*判断是否已经创建了有名管道，如果已经创建，则返回0否则返回非0的数*/
    {
        int ret = mkfifo((pipe_name + (_is_parent ? "_2" : "_1")).c_str(), 0777); /*创建有名管道,成功返回0,失败返回-1*/
        if (ret == -1) /*创建有名管道失败*/
        {
            logger::instance()->e(TAG, __LINE__, "mkfifo %d:%s", errno, strerror(errno));
            return -1;
        }
    }
    fd_R = open((pipe_name + (_is_parent ? "_1" : "_2")).c_str(), O_RDONLY | O_NONBLOCK);
    if (fd_R == -1) /*打开失败*/
    {
        logger::instance()->e(TAG, __LINE__, "open %d:%s", errno, strerror(errno));
        return -1;
    }
    if (!_is_parent && fd_W <= 0) {
        fd_W = open((pipe_name + (is_parent ? "_2" : "_1")).c_str(), O_WRONLY| O_NONBLOCK);
    }
#endif
    return 0;
}

void Pipe::Free() {
#ifdef _WIN32
#else
    if (fd_W > 0)
        close(fd_W); /*关闭有名管道*/
    fd_W = 0;
    if (fd_R > 0)
        close(fd_R); /*关闭有名管道*/
    fd_R = 0;
#endif
}

int Pipe::check_read_count(int timeout_ms) {
    if (fd_R <= 0)return -1;
    client.fd = fd_R;
    client.revents = 0;
#ifdef WIN32
    client.events = POLLIN;
    int poll_ret = WSAPoll(&client, 1, timeout_ms);
#else
    //    client.events = POLLIN | POLLPRI | POLLRDNORM;
    client.events = POLLIN;
    int poll_ret = poll(&client, 1, timeout_ms);
#endif
    if (poll_ret < 0) {
        if (errno != EINTR) {
            logger::instance()->e(TAG, __LINE__, "poll is not EINTR ;errno is %d ", errno);
            return poll_ret;
        }
        logger::instance()->w(TAG, __LINE__, "poll is EINTR");
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

int Pipe::read(std::vector<unsigned char> &data) {
    if (check_read_count(1) < 0)return 0;
    unsigned char buf[512] = {};
    do {
        int ret = 0;
#ifdef WIN32

#else
        ret = ::read(fd_R, buf, 512);
#endif
        if (ret <= 0)
            return ret;
        data.insert(data.end(), &buf[0], &buf[ret]);
        if (ret != 512) {
            break;
        }
    } while (true);
    return 0;
}

int Pipe::send(std::vector<unsigned char> data) {
    return send(data.data(), data.size());
}

int Pipe::send(const std::string &data) {
    return send((void *) data.c_str(), data.size());
}

int Pipe::send(void *data, int len) {
#ifdef WIN32
#else
    if (fd_W <= 0) {
        fd_W = open((pipe_name + (is_parent ? "_2" : "_1")).c_str(), O_WRONLY | O_NONBLOCK);
    }
    if (fd_W == -1) /*打开失败*/
    {
        // Free();
        logger::instance()->e(TAG, __LINE__, "open %d:%s", errno, strerror(errno));
        return -1;
    }
#endif
    int ret = 0;
#ifdef WIN32
#else
    ret = write(fd_W, data, len);
#endif
    return ret;
}
