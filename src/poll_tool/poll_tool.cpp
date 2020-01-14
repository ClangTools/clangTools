//
// Created by caesar on 2020/1/13.
//

#include "poll_tool.h"
#include <logger.h>
#ifdef __FILENAME__
const char *poll_tool::TAG = __FILENAME__;
#else
const char *poll_tool::TAG = "poll_tool";
#endif
poll_tool::poll_tool(int *fd) {
    this->fd = fd;
}

long int poll_tool::check_read_count(int timeout) {
    if (*fd <= 0) {
        logger::instance()->e(TAG, __LINE__, "fd is error");
        return -1;
    }
    client.fd = *fd;
    client.revents = 0;
#ifdef WIN32
    client.events = POLLIN;
    int poll_ret = WSAPoll(&client, 1, timeout);
#else
    // client.events = POLLIN | POLLPRI | POLLRDNORM;
    client.events = POLLIN;
    int poll_ret = poll(&client, 1, timeout);
#endif
    if (poll_ret < 0) {
        if (errno != EINTR) {
            logger::instance()->e(TAG, __LINE__, "poll is not EINTR ;errno is %d ", errno);
            return poll_ret;
        }
        logger::instance()->w(TAG, __LINE__, "poll is EINTR");
        return poll_ret;
    } else if (poll_ret == 0) {
        // logger::instance()->d(TAG, __LINE__, "poll is time out");
        return poll_ret;
    } else if (poll_ret > 0) {
        // logger::instance()->d(TAG, __LINE__, "%s POLLIN is %d", __FUNCTION__, client.fd);
        return poll_ret;
    }
    return 1;
}
