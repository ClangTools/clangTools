//
// Created by caesar on 2019/12/30.
//

#include "HTTP.h"
#include <regex>
#include <utility>
#include <logger.h>
#include <UTF8Url.h>
#include <socket.h>
#include <curl/curl.h>
#include <thread>

using namespace std;

#ifdef __FILENAME__
const char *HTTP::TAG = __FILENAME__;
#else
const char *HTTP::TAG = "HTTP";
#endif

std::string HTTP::url_regex = "([htps]{3,5})?(://)?([^/]+):?(\\d*)(/?[^?]*)\\?" "?([^?]*)";
#ifdef ENABLE_OPENSSL
SSL_CTX *HTTP::ctx = nullptr;
bool HTTP::is_ssl_inited = false;
std::mutex HTTP::init_ssl_mutex;

void HTTP::init_ssl() {
    std::unique_lock<std::mutex> lock(init_ssl_mutex);
    if (!is_ssl_inited) {
        is_ssl_inited = true;
        SSL_load_error_strings();
        ERR_load_crypto_strings();
        OpenSSL_add_all_algorithms();
        SSL_library_init();

        ctx = SSL_CTX_new(SSLv23_client_method());
        if (ctx == nullptr) {
            // printf("errored; unable to load context.\n");
            // ERR_print_errors_fp(stderr);
            // return -3;
            is_ssl_inited = false;
        }
    }
}

#endif
bool HTTP::is_curl_global_inited = false;
std::mutex HTTP::init_curl_global_init_mutex;

HTTP::HTTP(const std::string &full_url) {
    // global init
    std::unique_lock<std::mutex> lock(init_curl_global_init_mutex);
    if (!is_curl_global_inited) {
        is_curl_global_inited = true;
        curl_global_init(CURL_GLOBAL_ALL);
    }
    lock.unlock();

#ifdef ENABLE_OPENSSL
    init_ssl();
#endif
    logger::instance()->init_default();

    regex urlExpress(url_regex);

    smatch ms;
    if (regex_search(full_url, ms, urlExpress)) {
        protocol = ms.str(1);
        if (protocol.empty())protocol = "http";
        host = ms.str(3);
        port = ms.str(4);
        if (port.empty()) {
            port = protocol == "https" ? "443" : "80";
        }
        url = ms.str(5);
        if (url.empty())url = "/";
        string get_data = ms.str(6);
        init_key_value(GET, get_data);
    }
    /**
    logger::instance()->d(TAG, __LINE__, "protocol  : %s", protocol.c_str());
    logger::instance()->d(TAG, __LINE__, "host      : %s", host.c_str());
    logger::instance()->d(TAG, __LINE__, "url       : %s", url.c_str());
    logger::instance()->d(TAG, __LINE__, "GET");
    for (const auto &item : GET) {
        logger::instance()->d(TAG, __LINE__, "  %-10s : %s", item.first.c_str(), item.second.c_str());
    }
     */
}

void HTTP::init_key_value(std::map<std::string, std::string> &data, string value) {
    regex regExpress("&?([^=]+)=([^&]*)");
    string strText = std::move(value);
    smatch ms;
    while (regex_search(strText, ms, regExpress)) {
        data[ms.str(1)] = UTF8Url::Decode(ms.str(2));
        strText = ms.suffix().str();
    }
}

void HTTP::push_JSON(const CJsonObject &data) {
    method = "POST";
    ContentType = "application/json;charset=UTF-8";
    JSON = data;
}

void HTTP::push_POST(const std::map<std::string, std::string> &data) {
    method = "POST";
    ContentType = "application/x-www-form-urlencoded";
    for (const auto &item : data) {
        POST[item.first] = item.second;
    }
}

void HTTP::push_GET(const std::map<std::string, std::string> &data) {
    for (const auto &item : data) {
        GET[item.first] = item.second;
    }
}

void HTTP::push_HEADER(const std::map<std::string, std::string> &data) {
    for (const auto &item : data) {
        HEADER[item.first] = item.second;
    }
}

void HTTP::send(HTTP::callback cb, void *argv) {
    thread t([this](HTTP::callback cb, void *argv) {
        vector<unsigned char> data;
        int ret = send(data);
        cb(this, ret, data, argv);
    }, cb, argv);
    t.detach();
}

int HTTP::send(std::vector<unsigned char> &data) {
    string _url = protocol + "://" + host + (":" + port) + url;
    if (!GET.empty()) {
        _url += '?';
        for (const auto &item : GET) {
            _url += item.first + "=" + UTF8Url::Encode(item.second) + "&";
        }
        _url.erase(_url.end() - 1);
    }


    string getResponseStr;
    CURLcode res;

    if (curl != nullptr) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }

    // init curl
    curl = curl_easy_init();
    std::string header_string;
    string postParams;
    // res code
    if (curl) {
        // set params
        curl_easy_setopt(curl, CURLOPT_URL, _url.c_str()); // url
        if (method != "GET") {
            if (!POST.empty()) {
                for (const auto &item : POST) {
                    postParams += item.first + "=" + UTF8Url::Encode(item.second) + "&";
                }
                postParams.erase(postParams.end() - 1);
//                struct curl_httppost *post = NULL;
//                struct curl_httppost *last = NULL;
//                curl_formadd(&post, &last,CURLFORM_COPYNAME, "picture1", //此处表示要传的参数名
//                             CURLFORM_STREAM, "1.jpg",                               //此处表示图片文件的路径
//                             //CURLFORM_CONTENTTYPE, "image/jpeg",
//                             CURLFORM_END);
            } else if (!JSON.IsEmpty()) {
                postParams = JSON.ToString();
            }
            curl_easy_setopt(curl, CURLOPT_POST, 1); // post req
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postParams.c_str()); // params
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postParams.size()); // params
        }

        if (!USERPWD.empty())
            curl_easy_setopt(curl, CURLOPT_USERPWD, USERPWD.c_str());

        curl_slist *plist = curl_slist_append(nullptr,
                                              ("Content-Type: " + ContentType).c_str());
        if (!HEADER.empty()) {
            for (const auto &item : HEADER) {
                curl_slist_append(plist, (item.first + ": " + item.second).c_str());
            }
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, plist);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, protocol == "https"); // if want to use https
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, protocol == "https"); // set peer and host verify false
        curl_easy_setopt(curl, CURLOPT_VERBOSE, debug ? 1 : 0);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, req_reply);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, req_header_reply);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        // start req
        res = curl_easy_perform(curl);

        curl_slist_free_all(plist);
    }


    if (res != CURLE_OK) {
        logger::instance()->e(TAG, __LINE__, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
    } else {
        init_header(header_string.c_str(), header_string.size());
    }
    return -res;
}

size_t HTTP::req_reply(void *ptr, size_t size, size_t nmemb, void *stream) {
    ((vector<unsigned char> *) stream)->insert(((vector<unsigned char> *) stream)->end(),
                                               (unsigned char *) ptr,
                                               (unsigned char *) ptr + size * nmemb);
    return size * nmemb;
}

size_t HTTP::req_header_reply(void *ptr, size_t size, size_t nmemb, void *stream) {
    ((string *) stream)->append(
            (char *) ptr,
            (char *) ptr + size * nmemb);
    return size * nmemb;
}

#if false
int HTTP::send(std::vector<unsigned char> &data) {
    if (!GET.empty()) {
        url += '?';
        for (const auto &item : GET) {
            url += item.first + "=" + UTF8Url::Encode(item.second) + "&";
        }
        url.erase(url.end() - 1);
    }
    if (method != "GET" && (!JSON.IsEmpty() || !POST.empty())) {

    }

    string request_header = method + " " + url + " " + http_version;
    if (HEADER.find("Host") == HEADER.end())
        request_header += "\n" "Host: " + host;
    if (HEADER.find("User Agent") == HEADER.end())
        request_header += "\n" "User Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)";
    if (HEADER.find("Connection") == HEADER.end())
        request_header += "\n" "Connection: Close";
    {
        for (const auto &item : HEADER) {
            request_header += "\n" + item.first + ": " + item.second;
        }
    }
    request_header += "\n\n";

    if (this->protocol == "https") {
#ifdef ENABLE_OPENSSL
        if (ctx == nullptr) {
            logger::instance()->e(TAG, __LINE__, "openssl init is fail");
            return -2;
        }

        BIO *bio = BIO_new_ssl_connect(ctx);

        SSL *ssl;
        BIO_get_ssl(bio, &ssl);
        SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

        BIO_set_conn_hostname(bio, (host + ":" + protocol).c_str());

        if (BIO_do_connect(bio) <= 0) {
            BIO_free_all(bio);
            logger::instance()->e(TAG, __LINE__, "errored; unable to connect.");
            // ERR_print_errors_fp(stderr);
            return -2;
        }


        if (BIO_puts(bio, request_header.c_str()) <= 0) {
            BIO_free_all(bio);
            logger::instance()->e(TAG, __LINE__, "errored; unable to write.");
            // ERR_print_errors_fp(stderr);
            return -1;
        }

        char tmp_buf[1024 + 1];

        for (;;) {
            int len = BIO_read(bio, tmp_buf, 1024);
            if (len == 0) {
                break;
            } else if (len < 0) {
                if (!BIO_should_retry(bio)) {
                    logger::instance()->e(TAG, __LINE__, "errored; read failed.");
                    // ERR_print_errors_fp(stderr);
                    break;
                }
            } else {
                tmp_buf[len] = 0;
                data.insert(data.end(), &tmp_buf[0], &tmp_buf[len]);
            }
        }

        BIO_free_all(bio);
#else
        logger::instance()->e(TAG,__LINE__,"openssl is disable");
        return -1;
#endif
    } else {
        kekxv::socket httpClient(host.c_str(), port.c_str());
        httpClient.send(request_header, false);
        if (httpClient.isReady()) {
            if (httpClient.check_read_count(30 * 1000, false) <= 0) {
                return -1;
            }
            long int offset = 0;
            do {
                long int ret = 0;
                ret = httpClient.read(data, 0, false);
                if (ret <= 0) {
                    logger::instance()->e(TAG, __LINE__, "%ld", ret);
                }
                logger::instance()->i(TAG, __LINE__, "%ld", ret);
            } while (httpClient.check_read_count(1000, false) > 0);

        }
    }

    auto offset = init_header(reinterpret_cast<const char *>(data.data()), data.size());
    logger::instance()->d(TAG, __LINE__, "%lu : %lu", offset, data.size());
    data.erase(data.begin(), data.begin() + offset);
    logger::instance()->d(TAG, __LINE__, "%lu", data.size());
    return 0;
}
#endif

unsigned long int HTTP::init_header(const char *data, unsigned long int size) {
    unsigned long int offset = 0;
    int space_index = 0;
    http_version.clear();
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
                if (!http_version.empty() || data[offset] != ' ')
                    http_version.push_back(data[offset]);
                break;
            case 1:
                if (!http_code.empty() || data[offset] != ' ')
                    http_code.push_back(data[offset]);
                break;
            default:
                if (!http_code_message.empty() || data[offset] != ' ')
                    http_code_message.push_back(data[offset]);
                break;
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
                ResponseHeader[key] = value;
                if (l_key != key) {
                    ResponseHeader[l_key] = value;
                }
            }
            key = "";
            value = "";
            if (data[offset + 1] == '\r' && data[offset + 2] == '\n') {
                offset += 3;
                break;
            }
            continue;
        } else if (data[offset] == '\n') {
            space_index = 0;
            if (!key.empty() && !value.empty()) {
                string l_key = key;
                transform(l_key.begin(), l_key.end(), l_key.begin(), ::tolower);
                ResponseHeader[key] = value;
                if (l_key != key) {
                    ResponseHeader[l_key] = value;
                }
            }
            key = "";
            value = "";
            if (data[offset + 1] == '\n') {
                offset += 2;
                break;
            }
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
    logger::instance()->d(TAG, __LINE__, "header size:%ld", ResponseHeader.size());
    for (auto &item : ResponseHeader) {
        logger::instance()->d(TAG, __LINE__, "%-20s : %s", item.first.c_str(), item.second.c_str());
    }
    //*/

    return offset;
}

HTTP::~HTTP() {
    // release curl
    if (curl != nullptr) {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

