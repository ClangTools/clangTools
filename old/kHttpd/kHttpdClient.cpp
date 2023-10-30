//
// Created by caesar on 2019/12/1.
//
#ifdef WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>
#include <cstdio>

#else

#include <netinet/in.h>
#include <arpa/inet.h>
#include<unistd.h>

#endif

#include "kHttpdClient.h"
#include "UTF8Url.h"
#include "kHttpd.h"
#include <CGI/kCGI.h>
#include <string>
#include <utility>
#include <vector>
#include <cstring>

using namespace std;
#ifdef HAVE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
using namespace std::experimental;
#endif

#include <ctime>
#include <sstream>

using namespace std;

static map<string, string> kHttpdClient_HTTP_Content_Type = {
        {"*",     "application/octet-stream"},
        {".bmp",  "image/bmp"},
        {".ico",  "image/x-icon"},
        {".png",  "image/png"},
        {".jpg",  "image/*"},
        {".jpeg", "image/*"},
        {".tiff", "image/tiff"},
        {".ppt",  "application/x-ppt"},
        {".pdf",  "application/pdf"},
        {".mp2v", "video/mpeg"},
        {".wmv",  "video/x-ms-wmv"},
        {".mpeg", "video/mpg"},
        {".mp4",  "video/mpeg4"},
        {".avi",  "video/avi"},
        {".mp2",  "audio/mp2"},
        {".mp3",  "audio/mp3"},
        {".doc",  "application/msword"},
        {".html", "text/html"},
        {".htm",  "text/html"},
        {".css",  "text/css"},
        {".js",   "application/x-javascript"},
};

#ifdef __FILENAME__
const char *kHttpdClient::TAG = __FILENAME__;
#else
const char *kHttpdClient::TAG = "kHttpdClient";
#endif

logger *kHttpdClient::_logger = logger::instance();

kHttpdClient::kHttpdClient(kHttpd *parent, int fd) {
    init(parent, fd);
}

kHttpdClient::kHttpdClient(kHttpd *parent, int fd, const std::map<std::string, std::string> &header,
                           std::vector<unsigned char> data, unsigned long int split_index, bool is_split_n,
                           std::string method,
                           std::string url_path,
                           std::string _url_path_get,
                           std::map<std::string, std::string> GET,
                           std::string http_version,
                           kekxv::socket *_socket) {
    this->fd = fd;
    this->_socket = _socket;
    this->need_socket = false;

    init(parent, fd);
    this->url_path = std::move(url_path);
    this->_url_path_get = std::move(_url_path_get);
    this->method = std::move(method);
    this->http_version = std::move(http_version);
    this->header = header;
    this->GET = GET;
    /********* 读取body数据内容 *********/
    body_data.insert(body_data.end(), data.begin() + split_index, data.end());
}

void kHttpdClient::init(kHttpd *_parent, int _fd) {
    if (_logger->min_level != logger::log_rank_DEBUG) {
        _logger->min_level = logger::log_rank_DEBUG;
#ifdef _LOGGER_USE_THREAD_POOL_
        _logger->wait_show = false;
#endif
        _logger->console_show = true;
    }
    response_header["Access-Control-Allow-Origin"] = "*";
    response_header["Access-Control-Allow-Methods"] = "POST,GET,OPTIONS,DELETE";
    response_header["Access-Control-Allow-Credentials"] = "true";

    struct sockaddr_in remote_addr{};
    socklen_t sin_size = 0;
    getpeername(_fd, (struct sockaddr *) &remote_addr, &sin_size);
    this->parent = _parent;
    if (this->_socket == nullptr) {
        this->fd = _fd;
#ifdef ENABLE_OPENSSL
        this->_socket = new kekxv::socket(_fd,parent->sslCommon);
#else
        this->_socket = new kekxv::socket(_fd);
#endif
    }
}

kHttpdClient::~kHttpdClient() {
    if (need_socket) {
        delete this->_socket;
    }
}


int kHttpdClient::run() {
    vector<unsigned char> data;
    unsigned long int split_index = 0;
    bool is_split_n = true;

    long int ContentLength = 0;
    if (header.empty()) {
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
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d is errno: %d", size, fd, errno);
#ifdef WIN32
                //shutdown(fd, SD_BOTH);
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
        /********* 初始化http头 *********/
        init_header((const char *) data.data(), split_index, is_split_n);

        if (header.find("Content-Length") != header.end()) {
            ContentLength = stoll(header["Content-Length"]);
        } else if (header.find("content-length") != header.end()) {
            ContentLength = stoll(header["content-length"]);
        }
        /********* 读取body数据内容 *********/
        body_data.insert(body_data.end(), data.begin() + split_index, data.end());
    } else {
        if (header.find("Content-Length") != header.end()) {
            ContentLength = stoll(header["Content-Length"]);
        } else if (header.find("content-length") != header.end()) {
            ContentLength = stoll(header["content-length"]);
        }
    }
    if (ContentLength > 0) {
        while (body_data.size() < ContentLength) {
            vector<unsigned char> buffer;
            if (_socket->check_read_count(10) <= 0) {
                break;
            }
            auto size = this->_socket->read(buffer);
            if (0 == size) {//说明socket关闭
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d", size, fd);
#ifdef WIN32
                //                shutdown(fd, SD_BOTH);
                                closesocket(fd);
#else
                shutdown(fd, SHUT_RDWR);
                close(fd);
#endif
                fd = 0;
                return fd;
                break;
            } else if (0 > size && (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)) {
                _logger->w(TAG, __LINE__, "read size is %ld for socket: %d is errno:%d", size, fd, errno);
#ifdef WIN32
                //shutdown(fd, SD_BOTH);
                closesocket(fd);
#else
                shutdown(fd, SHUT_RDWR);
                close(fd);
#endif
                fd = 0;
                return fd;
                break;
            }
            body_data.insert(body_data.end(), buffer.begin(), buffer.end());
        };
    }


    /********* 输出body内容 *********/
    /*
    if (!body_data.empty()) {
        string post_data = (char *) body_data.data();
        _logger->d(TAG, __LINE__, "body:\n%s\n", (post_data).c_str());
        _logger->d(TAG, __LINE__, "body:\n%s\n", UTF8Url::Decode(post_data).c_str());
    }
    */

    try {
        if (0 > parent->check_host_path(this, header["Host"], method, url_path)) {
            if (!this->ResponseContent.empty()) {
                throw std::exception();
            }
            if (parent->web_root_path.empty()) {
                throw std::exception();
            } else {

#ifdef HAVE_EXPERIMENTAL_FILESYSTEM
                bool is_dirs = filesystem::is_directory(parent->web_root_path + url_path);
#else
                bool is_dirs = logger::is_dir(parent->web_root_path + url_path);
#endif
                if (is_dirs) {
                    url_path += (url_path[url_path.size() - 1] == '/' ? "" : "/") + string("index.html");
                }
#ifdef HAVE_EXPERIMENTAL_FILESYSTEM
                bool is_exists = filesystem::exists(parent->web_root_path + url_path);
#else
                bool is_exists = logger::exists(parent->web_root_path + url_path);
#endif
                if (!is_exists) {
                    throw std::exception();
                }

                if (url_path.find(".php") != string::npos) {
                    if (!parent->PhpSockPath.empty()) {
                        clangTools::kCGI php(parent->PhpSockPath);
                        kHttpd::RunPhpCGI(parent->web_root_path + url_path, php, this, this->response_header,
                                          this->ResponseContent);
                        this->ContentType = this->response_header["Content-type"];
                        this->response_header.erase("Content-type");
                    } else if (!parent->PhpIp.empty() && parent->PhpPort > 0) {
                        clangTools::kCGI php(parent->PhpIp, parent->PhpPort);
                        kHttpd::RunPhpCGI(parent->web_root_path + url_path, php, this, this->response_header,
                                          this->ResponseContent);
                        this->ContentType = this->response_header["Content-type"];
                        this->response_header.erase("Content-type");
                    } else {
                        throw std::exception();
                    }
                } else {

                    // Last-Modified: Fri, 01 Nov 2019 13:23:55 GMT
                    ifstream inFile(parent->web_root_path + url_path, ios::in | ios::binary);
                    if (!inFile) {
                        throw std::exception();
                    }
                    auto mtime = get_localtime(logger::get_mtime(parent->web_root_path + url_path));
                    response_header["Last-Modified"] = mtime;
                    response_header["Etag"] = mtime;
                    response_header["Accept-Ranges"] = "bytes";
                    if (header.find("if-modified-since") != header.end() && header["if-modified-since"] == mtime) {
                        this->response_code = HttpResponseCode::ResponseCode::NotModified;
                    } else {
                        unsigned char _buf[513];
                        while (inFile.read((char *) &_buf[0], 512 * sizeof(unsigned char)).gcount() > 0) {
                            ResponseContent.insert(ResponseContent.end(), &_buf[0], &_buf[inFile.gcount()]);
                        }
                        inFile.close();
                        string type = "*";

#ifdef HAVE_EXPERIMENTAL_FILESYSTEM
                        filesystem::path filepath(parent->web_root_path + url_path);
#ifdef WIN32
                        auto _ = filepath.extension();
                        type = _.string();
#else
                        type = filepath.extension();
#endif
#else
                        // TODO 获取后缀
                        type = logger::extension(parent->web_root_path + url_path);
#endif

                        if (kHttpdClient_HTTP_Content_Type.find(type) == kHttpdClient_HTTP_Content_Type.end())
                            type = "*";
                        this->ContentType = kHttpdClient_HTTP_Content_Type[type];
                    }
                }
            }
        }
    } catch (std::exception &e) {
        this->response_code = HttpResponseCode::ResponseCode::NotFound;
        if (this->ResponseContent.empty()) {
            string body = string("cann't found ") + url_path;
            this->ResponseContent.insert(ResponseContent.begin(), &body.c_str()[0], &body.c_str()[body.size()]);
        }
    }


    /********* 回复http 头 *********/
    send_header();
    /********* 回复http body内容 *********/
    send_body();

    if (response_code < 400) {
        _logger->i(TAG, __LINE__, "%-6s %3d %s ", method.c_str(), response_code,
                   _url_path_get.empty() ? url_path.c_str() : _url_path_get.c_str());
    } else if (response_code < 500) {
        _logger->w(TAG, __LINE__, "%-6s %3d %s ", method.c_str(), response_code,
                   _url_path_get.empty() ? url_path.c_str() : _url_path_get.c_str());
    } else {
        _logger->e(TAG, __LINE__, "%-6s %3d %s ", method.c_str(), response_code,
                   _url_path_get.empty() ? url_path.c_str() : _url_path_get.c_str());
    }
    return fd;
}

string kHttpdClient::get_localtime(time_t now) {
    char localtm[512] = {0};
    struct tm *timenow; //实例化tm结构指针
    if (now <= 0)
        time(&now);     //time函数读取现在的时间(国际标准时间非北京时间)，然后传值给now

#ifdef WIN32
    struct tm tmnow = { 0 };
    localtime_s(&tmnow, &now);
    char *daytime = asctime(&tmnow);
#else
    timenow = localtime(&now);
    char daytime[100];
    asctime_r(timenow, daytime);
#endif
    char year[20] = {0}, week[20] = {0}, day[20] = {0}, mon[20] = {0}, _time[20] = {0};
    sscanf(daytime, "%s %s %s %s %s", week, mon, day, _time, year);
    snprintf(localtm, 512, "%s, %s %s %s %s GMT", week, day, mon, year, _time);
    return localtm;
}

void kHttpdClient::init_header(const char *data, unsigned long int size, bool is_split_n) {
    unsigned long int offset = 0;
    int space_index = 0;
    if (method.empty()) {
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
    }
    if (method.empty())method = "GET";
    if (url_path.empty())url_path = "/";
    if (http_version.empty())http_version = "HTTP/1.0";

    auto _index = url_path.find('?');
    _url_path_get = url_path;
    if (_index != string::npos) {
        url_path = _url_path_get.substr(0, _index);
        auto _get = _url_path_get.substr(_index + 1);
        // UTF8Url::Decode()
        string key, value;
        bool is_value = false;
        for (char i: _get) {
            if (i == '&') {
                is_value = false;
                GET[key] = UTF8Url::Decode(value);
                key = "";
                value = "";
                continue;
            }
            if (i == '=') {
                is_value = true;
                continue;
            }
            if (is_value) {
                value.push_back(i);
            } else {
                key.push_back(i);
            }
        }
        if (!key.empty()) {
            GET[key] = UTF8Url::Decode(value);
            key = "";
            value = "";
        }
    }

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

void kHttpdClient::send_header() {
    string line_one = http_version + " " + to_string(response_code) + " " +
                      HttpResponseCode::get_response_code_string(response_code);
    /**
    _logger->d(TAG, __LINE__, "%s", line_one.c_str());
    */
    line_one.push_back('\n');
    this->_socket->send(line_one);
    response_header["Content-Type"] = ContentType;
    response_header["Date"] = get_localtime();
    response_header["Content-Length"] = to_string(ResponseContent.size());
    for (auto &item: response_header) {
        string line = item.first + ": " + item.second;
        /**
        _logger->d(TAG, __LINE__, "%s", line.c_str());
        */
        line.push_back('\n');
        this->_socket->send(line);
    }
    this->_socket->send("\n");
}

void kHttpdClient::send_body() {
    auto len = ResponseContent.size();
    long int offset = 0;
    while (offset < len) {
        auto ret = this->_socket->send(ResponseContent.data(), offset, len - offset);
        if (0 >= ret)break;
        offset += ret;
    }
}
