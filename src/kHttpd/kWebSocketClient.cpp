//
// Created by caesar on 2019/12/11.
//

#ifdef WIN32

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mstcpip.h>
#include <cstdio>

#else

#include <netinet/in.h>
#include <arpa/inet.h>
#include<unistd.h>

#ifdef ENABLE_OPENSSL
#include <openssl/sha.h>
#include <openssl/crypto.h>
#endif
#endif

#include "kWebSocketClient.h"
#include <kHttpdClient.h>
#include "UTF8Url.h"
#include "kHttpd.h"
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <base64.h>
#include <SHA1.h>


#ifndef SHA_DIGEST_LENGTH
#define SHA_DIGEST_LENGTH 20
#endif

using namespace std;


#ifdef __FILENAME__
const char *kWebSocketClient::TAG = __FILENAME__;
#else
const char *kWebSocketClient::TAG = "kWebSocketClient";
#endif

logger *kWebSocketClient::_logger = logger::instance();

kWebSocketClient::kWebSocketClient(kHttpd *parent, int fd) {
    if (_logger->min_level != logger::log_rank_DEBUG) {
        _logger->min_level = logger::log_rank_DEBUG;
#ifdef _LOGGER_USE_THREAD_POOL_
        _logger->wait_show = false;
#endif
        _logger->console_show = true;
    }

    struct sockaddr_in remote_addr{};
    socklen_t sin_size = 0;
    getpeername(fd, (struct sockaddr *) &remote_addr, &sin_size);
    this->parent = parent;
    this->fd = fd;
#ifdef ENABLE_OPENSSL
    this->_socket = new kekxv::socket(fd,parent->sslCommon);
#else
    this->_socket = new kekxv::socket(fd);
#endif
}

kWebSocketClient::~kWebSocketClient() {
    delete this->_socket;
}


int kWebSocketClient::run() {
    vector<unsigned char> data;
    unsigned long int split_index = 0;
    bool is_split_n = true;
    /********* 读取数据内容 *********/
    do {
        vector<unsigned char> buffer;
        if (_socket->check_read_count(3 * 1000) <= 0) {
            break;
        }
        auto size = this->_socket->read(buffer);
        if (size == 0) {
            if (_socket->check_read_count(3 * 1000) <= 0) {
                break;
            } else {
                size = this->_socket->read(buffer);
            }
        }
        // _logger->d(TAG,__LINE__,"%s",buffer.data());
        if (0 == size) {//说明socket关闭
            _logger->w(TAG, __LINE__, "read size is %ld for socket: %d", size, fd);
#ifdef WIN32
            // shutdown(fd, SD_BOTH);
            closesocket(fd);
#else
            shutdown(fd, SHUT_RDWR);
            close(fd);
#endif
            return 0;
        } else if (0 > size && (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)) {
            _logger->w(TAG, __LINE__, "read size is %ld for socket: %d is errno:", size, fd, errno);
#ifdef WIN32
            shutdown(fd, SD_BOTH);
            closesocket(fd);
#else
            shutdown(fd, SHUT_RDWR);
            close(fd);
#endif
            return 0;
        }
        if (!data.empty()) {
            if (data.size() >= 2 && data[data.size() - 2] == '\r' && data[data.size() - 1] == '\n') {
                if (buffer[0 + 0] == '\r' && buffer[0 + 1] == '\n') {
                    split_index = data.size() + 2;
                    is_split_n = false;
                }
            } else if (data[data.size() - 1] == '\n') {
                if (buffer[0] == '\n') {
                    split_index = data.size() + 1;
                    is_split_n = true;
                }
            }
        }
        if (split_index == 0) {
            for (long int i = 0; i < size - 3; i++) {
                if (buffer[i] == '\r' && buffer[i + 1] == '\n') {
                    if (buffer[i + 2] == '\r' && buffer[i + 3] == '\n') {
                        split_index = data.size() + i + 4;
                        is_split_n = false;
                    }
                } else if (buffer[i] == '\n') {
                    if (buffer[i + 1] == '\n') {
                        split_index = data.size() + i + 2;
                        is_split_n = true;
                    }
                }
            }
        }
        data.insert(data.end(), buffer.begin(), buffer.end());

    } while (split_index == 0);
    if (data.empty())return -1;
    /********* 初始化http头 *********/
    init_header((const char *) data.data(), split_index, is_split_n);

    string upgrade, connection;
    if (header.find("upgrade") != header.end()) {
        upgrade = logger::trim(header["upgrade"]);
        transform(upgrade.begin(), upgrade.end(), upgrade.begin(), ::tolower);
    }
    if (header.find("connection") != header.end()) {
        connection = logger::trim(header["connection"]);
        transform(connection.begin(), connection.end(), connection.begin(), ::tolower);
    }
    // _logger->i(TAG, __LINE__, "%s", SecWebSocketAccept.c_str());
    if ((connection != string("upgrade")) || (upgrade != string("websocket"))) {
        return kHttpdClient(parent, fd, header, data, split_index, is_split_n, method, url_path, http_version,
                            _socket).run();
    }

    SecWebSocketKey = logger::trim(header["sec-websocket-key"]);

    string key = SecWebSocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    unsigned char md[SHA_DIGEST_LENGTH];
    // SHA1((unsigned char *) key.c_str(), key.size(), md);
    CSHA1::SHA1((unsigned char *) key.c_str(), key.size(), md);

    SecWebSocketAccept = base64_encode(md, 20);// 升级key的算法
    send_header();

    auto ret = _run();
    this->parent->check_host_path(this, 8, vector<unsigned char>());
    return ret;
}

string kWebSocketClient::get_localtime(time_t now) {
    char localtm[512] = {0};
    struct tm *timenow; //实例化tm结构指针
    if (now <= 0)
        time(&now);     //time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now

#ifdef WIN32
    struct tm tmnow = {0};
    localtime_s(&tmnow, &now);
    char *daytime = asctime(&tmnow);
#else
    timenow = localtime(&now);
    char daytime[100];
    asctime_r(timenow, daytime);
#endif

    char year[20] = {0}, week[20] = {0}, day[20] = {0}, mon[20] = {0}, _time[20] = {0};
    sscanf(daytime, "%s %s %s %s %s", week, mon, day, _time, year);
    sprintf(localtm, "%s, %s %s %s %s GMT", week, day, mon, year, _time);
    return localtm;
}

void kWebSocketClient::init_header(const char *data, unsigned long int size, bool is_split_n) {
    unsigned long int offset = 0;
    int space_index = 0;
    for (; offset < size; offset++) {
        if (data[offset] == '\r' && data[offset + 1] == '\n') {
            offset += 2;
            break;
        } else if (data[offset] == '\n') {
            offset++;
            break;
        } else if (data[offset] == ' ') {
            space_index++;
        }
        switch (space_index) {
            case 0:
                if (!method.empty() || data[offset] != ' ')
                    method.push_back(data[offset]);
                break;
            case 1:
                if (!url_path.empty() || data[offset] != ' ')
                    url_path.push_back(data[offset]);
                break;
            default:
                if (!http_version.empty() || data[offset] != ' ')
                    http_version.push_back(data[offset]);
                break;
        }
    }
    if (method.empty())method = "GET";
    if (url_path.empty())url_path = "/";
    if (http_version.empty())http_version = "HTTP/1.1";

    space_index = 0;
    string key, value;
    for (; offset < size; offset++) {
        if (data[offset] == '\r' && data[offset + 1] == '\n') {
            offset += 1;
            space_index = 0;
            if (!key.empty() && !value.empty()) {
                string l_key = key;
                transform(l_key.begin(), l_key.end(), l_key.begin(), ::tolower);
                header[key] = value;
                if (l_key != key) {
                    header[l_key] = value;
                }
            }
            key = "";
            value = "";
            continue;
        } else if (data[offset] == '\n') {
            space_index = 0;
            if (!key.empty() && !value.empty()) {
                string l_key = key;
                transform(l_key.begin(), l_key.end(), l_key.begin(), ::tolower);
                header[key] = value;
                if (l_key != key) {
                    header[l_key] = value;
                }
            }
            key = "";
            value = "";
            continue;
        } else if (data[offset] == ':') {
            space_index++;
            if (data[offset + 1] == ' ') {
                offset++;
            }
        }
        if (space_index == 0) {
            key.push_back(data[offset]);
        } else {
            value.push_back(data[offset]);
        }
    }

    /***
    _logger->d(TAG, __LINE__, "header size:%ld", header.size());
    for (auto &item : header) {
        _logger->d(TAG, __LINE__, "%s : %s", item.first.c_str(), item.second.c_str());
    }
    //*/

}

void kWebSocketClient::send_header() {
    string line_one = http_version + " " + to_string(response_code) + " " +
                      HttpResponseCode::get_response_code_string(response_code);
    /**
    _logger->d(TAG, __LINE__, "%s", line_one.c_str());
    */
    line_one.push_back('\n');
    this->_socket->send(line_one);
    response_header["Upgrade"] = "WebSocket";
    // response_header["Sec-WebSocket-key"] = "WebSocket";
    response_header["Sec-WebSocket-Accept"] = SecWebSocketAccept;
    response_header["Sec-WebSocket-Version"] = "13";
    response_header["Connection"] = "Upgrade";
    for (auto &item : response_header) {
        string line = item.first + ": " + item.second;
        /**
        _logger->d(TAG, __LINE__, "%s", line.c_str());
        */
        line.push_back('\n');
        this->_socket->send(line);
    }
    this->_socket->send("\n");
}

int kWebSocketClient::_run() {
    unsigned long long next = 0;
    unsigned char FIN = 1;
    do {
        vector<unsigned char> data;
        vector<unsigned char> last_buffer;
        int ret_type = 0;
        do {
            int ret = 1;
            vector<unsigned char> buffer;
            if (last_buffer.empty()) {
                if (_socket->check_read_count(-1) <= 0) {
                    break;
                }
                auto size = this->_socket->read(buffer);
                if (0 == size) {//说明socket关闭
                    _logger->w(TAG, __LINE__, "read size is %ld for socket: %d", size, fd);
#ifdef WIN32
                    // shutdown(fd, SD_BOTH);
                    closesocket(fd);
#else
                    shutdown(fd, SHUT_RDWR);
                    close(fd);
#endif

                    return 0;
                } else if (0 > size && (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)) {
                    _logger->w(TAG, __LINE__, "read size is %ld for socket: %d is errno:%d", size, fd, errno);
#ifdef WIN32
                    // shutdown(fd, SD_BOTH);
                    closesocket(fd);
#else
                    shutdown(fd, SHUT_RDWR);
                    close(fd);
#endif
                    return 0;
                }
            } else {
                buffer.insert(buffer.begin(), last_buffer.begin(), last_buffer.end());
                last_buffer.clear();
            }
            ret = parse(buffer, FIN, next);
            if (ret < 0)
                return 0;
            data.insert(data.end(), buffer.begin(), buffer.begin() + next);
            if (next < buffer.size()) {
                last_buffer.insert(last_buffer.end(), buffer.begin() + next, buffer.end());
            }
            if (ret > 0) {
                ret_type = ret;
            }
        } while (FIN == 0);

        if (ret_type == 8) {
            break;
        }

        this->parent->check_host_path(this, ret_type, data);

    } while (true);
    return fd;
}

int kWebSocketClient::parse(std::vector<unsigned char> &data, unsigned char &FIN, unsigned long long &next) {
    next = 0;
    FIN = (data[0] >> 7) & 0b1;
    unsigned char RSV1 = (data[0] >> 6) & 0b1;
    unsigned char RSV2 = (data[0] >> 5) & 0b1;
    unsigned char RSV3 = (data[0] >> 4) & 0b1;
    unsigned char OPCODE = (data[0]) & 0b1111;
    unsigned char $need_masks = (data[1] >> 7) & 0b1;
    unsigned char $len = data[1] & 127;

    unsigned char $masks[4];
    if ($len == 126) {
        unsigned short $len_bit;
        memcpy(&((unsigned char *) &$len_bit)[0], &data.data()[3], 1);
        memcpy(&((unsigned char *) &$len_bit)[1], &data.data()[2], 1);
        while (data.size() + 8 < $len_bit) {
            std::vector<unsigned char> $buffer1;
            if (_socket->check_read_count(-1) <= 0) {
                break;
            }
            auto $bytes = this->_socket->read($buffer1);
            if (0 == $bytes) {//说明socket关闭
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d", $bytes, fd);
#ifdef WIN32
                // shutdown(fd, SD_BOTH);
                closesocket(fd);
#else
                shutdown(fd, SHUT_RDWR);
                close(fd);
#endif
                return -1;
            } else if (0 > $bytes && (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)) {
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d is errno:%d", $bytes, fd, errno);
#ifdef WIN32
                //shutdown(fd, SD_BOTH);
                closesocket(fd);
#else
                shutdown(fd, SHUT_RDWR);
                close(fd);
#endif
                return -1;
            }
            if ($bytes < 7) {
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d", $bytes, fd);
#ifdef WIN32
                shutdown(fd, SD_BOTH);
                closesocket(fd);
#else
                shutdown(fd, SHUT_RDWR);
                close(fd);
#endif
                return -1;
            }
            data.insert(data.end(), $buffer1.begin(), $buffer1.end());
        }

        if ($need_masks) {
            memcpy($masks, &data.data()[4], 4);
            data.erase(data.begin(), data.begin() + 8);
        } else {
            data.erase(data.begin(), data.begin() + 4);
        }
        next = $len_bit;
    } else if ($len == 127) {
        unsigned long long $len_bit = 0;
        memcpy(&$len_bit, &data.data()[2], 8);
        for (int i = 0; i < 8; i++)
            memcpy(&((unsigned char *) &$len_bit)[i], &data.data()[2 + 7 - i], 1);
        //memcpy(&((unsigned char *) &$len_bit)[1], &data.data()[2], 1);

        while (data.size() + 14 < $len_bit) {
            std::vector<unsigned char> $buffer1;
            if (_socket->check_read_count(-1) <= 0) {
                break;
            }
            auto $bytes = this->_socket->read($buffer1);
            if (0 == $bytes) {//说明socket关闭
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d", $bytes, fd);
#ifdef WIN32
                // shutdown(fd, SD_BOTH);
                closesocket(fd);
#else
                shutdown(fd, SHUT_RDWR);
                close(fd);
#endif
                return -1;
            } else if (0 > $bytes && (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)) {
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d is errno:%d", $bytes, fd, errno);
#ifdef WIN32
                //shutdown(fd, SD_BOTH);
                closesocket(fd);
#else
                shutdown(fd, SHUT_RDWR);
                close(fd);
#endif
                return -1;
            }
            if ($bytes < 7) {
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d", $bytes, fd);
#ifdef WIN32
                //shutdown(fd, SD_BOTH);
                closesocket(fd);
#else
                shutdown(fd, SHUT_RDWR);
                close(fd);
#endif
                return -1;
            }
            data.insert(data.end(), $buffer1.begin(), $buffer1.end());
        }

        if ($need_masks) {
            memcpy($masks, &data.data()[10], 4);
            data.erase(data.begin(), data.begin() + 14);
        } else {
            data.erase(data.begin(), data.begin() + 10);
        }
        next = $len_bit;
    } else {
        if ($need_masks) {
            memcpy($masks, &data.data()[2], 4);
            data.erase(data.begin(), data.begin() + 6);
        } else {
            data.erase(data.begin(), data.begin() + 2);
        }
        next = $len;
    }
    if ($need_masks) {
        for (long int $index = 0; $index < next; $index++) {
            data[$index] = data[$index] ^ $masks[$index % 4];
        }
    }
    return OPCODE;
}


int kWebSocketClient::build(std::vector<unsigned char> &send_data, const std::vector<unsigned char> &data,
                            unsigned char type,
                            bool $need_masks) {
    unsigned char _type = 0b10000000;
    _type += type;
    send_data.push_back(_type);
    unsigned long long len = data.size();

    unsigned char $masks[4];
    if ($need_masks) {
        srand((int) time(nullptr));
        unsigned int mask = (rand() % (0xFFFFFFFF - 0xFFFF) + 0xFFFF);
        memcpy($masks, &mask, 4);
    }
    if (len < 126) {
        send_data.push_back(len + ($need_masks ? 0b10000000 : 0));
    } else if (len < 65025) {
        send_data.push_back(0x7e + ($need_masks ? 0b10000000 : 0));
        for (int i = 1; i >= 0; i--) {
            send_data.push_back((len >> (i * 8)) & 0xFF);
        }
    } else {
        send_data.push_back(0x7f + ($need_masks ? 0b10000000 : 0));
        for (int i = 7; i >= 0; i--) {
            send_data.push_back((len >> (i * 8)) & 0xFF);
        }
    }

    if ($need_masks) {
        send_data.insert(send_data.end(), &$masks[0], &$masks[4]);
        for (long int $index = 0; $index < len; $index++) {
            send_data.push_back(data[$index] ^ $masks[$index % 4]);
        }
    } else {
        send_data.insert(send_data.end(), data.begin(), data.end());
    }
    //return _socket->send(send_data);
    return send_data.size();
}


int kWebSocketClient::send(const std::vector<unsigned char> &data, unsigned char type) {
    std::vector<unsigned char> send_data;
    if (build(send_data, data, type, false) > 0) {
        /*
        std::vector<unsigned char> _buffer = send_data;
        unsigned long long _next = 0;
        unsigned char _FIN = 1;
        int _ret = parse(_buffer, _FIN, _next);
        */
        std::unique_lock<std::mutex> lock{this->m_send_lock};
        return this->_socket->send(send_data);
    }
    return -1;
}

int kWebSocketClient::send(const std::string &data) {
    std::vector<unsigned char> send_data;
    send_data.insert(send_data.end(), data.data(), data.data() + data.size());
    return send(send_data, 1);
}
