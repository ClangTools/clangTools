#include "kCGI.h"
#include "fastcgi.h"

#include <cstring>
#include <cstdio>
#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mstcpip.h>
#include <stdio.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#endif
#include <regex>
#include <logger.h>

using namespace clangTools;
int kCGI::RequestId = 1;

kCGI::kCGI(const string &ip, int port) {
    int rc;
    int fd;
    struct sockaddr_in server_address{};

    fd = socket(AF_INET, SOCK_STREAM, 0);
//    assert(fd > 0);
    if (fd <= 0)throw kCGIException(1);

    memset(&server_address,0, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip.c_str());
    server_address.sin_port = htons(port);

    rc = connect(fd, (struct sockaddr *) &server_address, sizeof(server_address));
//    assert(rc >= 0);
    if (rc < 0)throw kCGIException(2);

    this->SockFd = fd;
    this->requestId = kCGI::RequestId++;
}

kCGI::kCGI(const string &SockFilePath) {
#ifdef WIN32
#else
    int rc;
    int fd;
    struct sockaddr_un s_un{};
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
//    assert(fd > 0);
    if (fd <= 0)throw kCGIException(0);
    s_un.sun_family = AF_UNIX;
    strcpy(s_un.sun_path, SockFilePath.c_str());
    rc = connect(fd, (struct sockaddr *) &s_un, sizeof(s_un));
//    assert(rc >= 0);
    if (rc < 0)throw kCGIException(1);

    this->SockFd = fd;
    this->requestId = kCGI::RequestId++;
#endif
}

kCGI::~kCGI() {
    close(this->SockFd);
}

int kCGI::sendStartRequestRecord() {
    int rc;
    FCGI_BeginRequestRecord beginRecord;

    beginRecord.header = makeHeader(FCGI_BEGIN_REQUEST, this->requestId, sizeof(beginRecord.body), 0);
    beginRecord.body = makeBeginRequestBody(FCGI_RESPONDER, 0);

    rc = write(this->SockFd, (char *) &beginRecord, sizeof(beginRecord));
    if (rc != sizeof(beginRecord)) {
        return 0;
    }

    return 1;
}

FCGI_Header kCGI::makeHeader(int type, int request,
                             int contentLength, int paddingLength) {
    FCGI_Header header;
    header.version = FCGI_VERSION_1;
    header.type = (unsigned char) type;
    /* 两个字段保存请求ID */
    header.requestIdB1 = (unsigned char) ((this->requestId >> 8) & 0xff);
    header.requestIdB0 = (unsigned char) (this->requestId & 0xff);
    /* 两个字段保存内容长度 */
    header.contentLengthB1 = (unsigned char) ((contentLength >> 8) & 0xff);
    header.contentLengthB0 = (unsigned char) (contentLength & 0xff);
    /* 填充字节的长度 */
    header.paddingLength = (unsigned char) paddingLength;
    /* 保存字节赋为 0 */
    header.reserved = 0;
    return header;
}

FCGI_BeginRequestBody kCGI::makeBeginRequestBody(int role, int keepConnection) {
    FCGI_BeginRequestBody body;
    /* 两个字节保存期望 php-fpm 扮演的角色 */
    body.roleB1 = (unsigned char) ((role >> 8) & 0xff);
    body.roleB0 = (unsigned char) (role & 0xff);
    /* 大于0长连接，否则短连接 */
    body.flags = (unsigned char) ((keepConnection) ? FCGI_KEEP_CONN : 0);
    memset(&body.reserved, 0, sizeof(body.reserved));
    return body;
}

int kCGI::makeNameValueBody(const char *name, int nameLen,
                            const char *value, int valueLen,
                            unsigned char *bodyBuffPtr, int *bodyLenPtr) {
    /* 记录 body 的开始位置 */
    unsigned char *startBodyBuffPtr = bodyBuffPtr;

    /* 如果 nameLen 小于128字节 */
    if (nameLen < 128) {
        *bodyBuffPtr++ = (unsigned char) nameLen;    //nameLen用1个字节保存
    } else {
        /* nameLen 用 4 个字节保存 */
        *bodyBuffPtr++ = (unsigned char) ((nameLen >> 24) | 0x80);
        *bodyBuffPtr++ = (unsigned char) (nameLen >> 16);
        *bodyBuffPtr++ = (unsigned char) (nameLen >> 8);
        *bodyBuffPtr++ = (unsigned char) nameLen;
    }

    /* valueLen 小于 128 就用一个字节保存 */
    if (valueLen < 128) {
        *bodyBuffPtr++ = (unsigned char) valueLen;
    } else {
        /* valueLen 用 4 个字节保存 */
        *bodyBuffPtr++ = (unsigned char) ((valueLen >> 24) | 0x80);
        *bodyBuffPtr++ = (unsigned char) (valueLen >> 16);
        *bodyBuffPtr++ = (unsigned char) (valueLen >> 8);
        *bodyBuffPtr++ = (unsigned char) valueLen;
    }

    /* 将 name 中的字节逐一加入body中的buffer中 */
    for (size_t i = 0; i < strlen(name); i++) {
        *bodyBuffPtr++ = name[i];
    }

    /* 将 value 中的值逐一加入body中的buffer中 */
    for (size_t i = 0; i < strlen(value); i++) {
        *bodyBuffPtr++ = value[i];
    }

    /* 计算出 body 的长度 */
    *bodyLenPtr = bodyBuffPtr - startBodyBuffPtr;
    return 1;
}

int kCGI::sendParams(const char *name, const char *value) {
    int rc;

    unsigned char bodyBuff[PARAMS_BUFF_LEN];

    memset(bodyBuff, 0,sizeof(bodyBuff));

    /* 保存 body 的长度 */
    int bodyLen;

    /* 生成 PARAMS 参数内容的 body */
    makeNameValueBody(name, strlen(name), value, strlen(value), bodyBuff, &bodyLen);

    FCGI_Header nameValueHeader;
    nameValueHeader = makeHeader(FCGI_PARAMS, this->requestId, bodyLen, 0);

    int nameValueRecordLen = bodyLen + FCGI_HEADER_LEN;
    char *nameValueRecord = new char[nameValueRecordLen];

    /* 将头和body拷贝到一块buffer 中只需调用一次write */
    memcpy(nameValueRecord, (char *) &nameValueHeader, FCGI_HEADER_LEN);
    memcpy(nameValueRecord + FCGI_HEADER_LEN, bodyBuff, bodyLen);

    rc = write(this->SockFd, nameValueRecord, nameValueRecordLen);
    delete []nameValueRecord;
    if (rc != nameValueRecordLen) {
        return 0;
    }

    return 1;
}

int kCGI::sendEndRequestRecord() {
    int rc;
    FCGI_Header endHeader;
    endHeader = makeHeader(FCGI_PARAMS, this->requestId, 0, 0);
    rc = write(this->SockFd, (char *) &endHeader, FCGI_HEADER_LEN);
    return (rc == FCGI_HEADER_LEN);
}

char *kCGI::findStartHtml(char *content) {
    for (; *content != '\0'; content++) {
        if (*content == '\r' && *(content + 1) == '\n')
            if (*(content + 2) == '\r' && *(content + 3) == '\n')
                return content + 4;
    }
    return nullptr;
}


size_t kCGI::ReadFromPhp(map<string, string> &header, vector<unsigned char> &data) {
    FCGI_Header responderHeader;
    unsigned char content[CONTENT_BUFF_LEN];

    int contentLen;
    char tmp[256];    //用来暂存padding字节
    int ret;
    kCgiData kData;
    kCgiData *kD = &kData;
    /* 先将头部 8 个字节读出来 */
    while (read(this->SockFd, &responderHeader, FCGI_HEADER_LEN) > 0) {
        if (responderHeader.type == FCGI_STDOUT) {
            /* 获取内容长度 */
            contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
            memset(content, 0,CONTENT_BUFF_LEN);

            ret = Read(content, contentLen);
            if (ret > 0) {
                kD->Put(content, ret);
                kD->next = new kCgiData();
                kD = kD->next;
            }
//            getHtmlFromContent(content);
            /* 跳过填充部分 */
            if (responderHeader.paddingLength > 0) {
//                printf("-----> %d\n",ret);
                ret = Read(tmp, responderHeader.paddingLength);
            }
        } //end of type FCGI_STDOUT
        else if (responderHeader.type == FCGI_STDERR) {
            contentLen = (responderHeader.contentLengthB1 << 8) + (responderHeader.contentLengthB0);
            memset(content, 0,CONTENT_BUFF_LEN);

            ret = Read(content, contentLen);
            if (ret > 0) {
                kD->Put(content, ret);
                kD->next = new kCgiData();
                kD = kD->next;
            }
//            fprintf(stdout, "error:%s\n", content);
            /* 跳过填充部分 */
            if (responderHeader.paddingLength > 0) {
                ret = Read(tmp, responderHeader.paddingLength);
            }
        } else if (responderHeader.type == FCGI_END_REQUEST) {
            FCGI_EndRequestBody endRequest;
            ret = read(this->SockFd, &endRequest, sizeof(endRequest));
        }
    }
    kData.ToVector(data);
    size_t offset = 0;
    for (size_t i = 0; i < data.size(); i++) {
        char *p = (char *) &data[i];
        if (*p == '\r' && *(p + 1) == '\n')
            if (*(p + 2) == '\r' && *(p + 3) == '\n') {
                offset = p + 4 - (char *) (data.data());
                break;
            }
    }
    if (offset == 0)return 1;
    char *da = new char[offset + 1 - 4];
    memcpy(da, data.data(), offset - 4);
    da[offset - 4] = 0;
    if (data[offset] != '<') {
        int ret = offset;
    }
    string _header(da);

    //正则表达式
    string regex_str("([^:]+):([^\r\n]+)\r?\n?");
    regex pattern1(regex_str, regex::icase);

    //迭代器声明
    string::const_iterator iter = _header.begin();
    string::const_iterator iterEnd = _header.end();
    smatch result;
    //正则查找
    while (std::regex_search(iter, iterEnd, result, pattern1)) {
        string t = result[1];
        string t1 = result[2];
        header[logger::trim(t)] = logger::trim(t1);
        iter = result[0].second; //更新搜索起始位置
    }

    delete[]da;
    data.erase(data.begin(), data.begin() + offset);
    return 1;
}

void kCGI::getHtmlFromContent(char *content) {
    /* 保存html内容开始位置 */
    char *pt;

    /* 读取到的content是html内容 */
//    if(flag == 1){
    printf("%s", content);
//    } else {
//        if((pt = findStartHtml(content)) != NULL){
//            c->flag_ = 1;
//            for(char *i = pt; *i != '\0'; i++){
//                printf("%c",*i);
//            }
//        }
//    }
}

size_t kCGI::Read(void *tmp, size_t Len) {
    size_t len = 0;
    while (len < Len) {
        int ret = read(this->SockFd, tmp, Len - len);
        if (ret == -1)return 0;
        len += ret;
    }
    return len;
}

kCgiData::~kCgiData() {
    if (next != nullptr) {
        delete next;
    }
    next = nullptr;
    if (Data != nullptr) {
        delete Data;
    }
    Data = nullptr;
}

kCgiData::kCgiData(unsigned char *data, size_t len, size_t offset) {
    Put(data, len, offset);
}

kCgiData::kCgiData() = default;

void kCgiData::Put(unsigned char *data, size_t len, size_t offset) {
    Data = new unsigned char[len];
    memcpy(Data, &data[offset], len);
    DataLen = len;
}

void kCgiData::Print() {
    if (this->Data != nullptr)printf("%s", this->Data);
    if (this->next != nullptr) {
        this->next->Print();
    }
}

void kCgiData::ToVector(vector<unsigned char> &data) {
    if (this->Data == nullptr)return;
    for (size_t i = 0; i < this->DataLen; i++) {
        data.push_back(this->Data[i]);
    }
    if (this->next != nullptr) {
        this->next->ToVector(data);
    }
}
