//
// Created by caesar on 2020/1/15.
//

#include "Popen.h"
#include <logger.h>
#include <poll_tool.h>

#ifdef _WIN32
#else

#include <fcntl.h>

#endif

using namespace std;


#ifdef __FILENAME__
const char *Popen::TAG = __FILENAME__;
#else
const char *Popen::TAG = "Popen";
#endif

Popen::Popen(const char *command, const std::vector<std::string> &args) {
    command_line = command;
    for (const auto &item : args) {
        command_line += " " + string("\"") + item + "\"";
    }
    logger::instance()->i(TAG, __LINE__, "%s", command_line.c_str());
}

Popen::~Popen() {
    stop();
}

bool Popen::start() {
#ifdef _WIN32
    fp = _popen(command_line.c_str(), "r+");
#else
    fp = popen(command_line.c_str(), "r+");
#endif
    if (!fp) {
        return false;
    }
#ifdef _WIN32
#else
    fd = fileno(fp);
    fcntl(fd, F_SETFL, O_NONBLOCK);
#endif
    return true;
}

bool Popen::stop() {
    if (!fp) {
#ifdef _WIN32
        _pclose(fp);
#else
        pclose(fp);
#endif
        fp = nullptr;
    }
    fd = 0;
    return true;
}

int Popen::read(string &data, int timeout) {
    if (!fp)return -1;

    if (poll_tool::check_read_count(fd, timeout) <= 0) {
        return 0;
    }
    char buf[1024] = {};
    do {
        int ret = ::read(fd, buf, sizeof(buf) - 1);
        if (ret == -1 && errno == EAGAIN) {
            break;
        } else if (ret > 0) {
            buf[ret] = 0;
            data += buf;
        } else {
            if (data.empty())
                return -1;
            else {
                break;
            }
        }
    } while (poll_tool::check_read_count(fd, timeout) > 0);

    return data.size();
}

int Popen::write(const std::string &data) {
    if (!fp)return -1;
    return ::write(fd, data.c_str(), data.size());
}
