//
// Created by caesar on 2019-07-25.
//

#ifndef KHTTPD_KCGI_H
#define KHTTPD_KCGI_H

#include <string>
#include <cstdio>
#include "fastcgi.h"
#include <map>
#include <vector>

using namespace std;
namespace clangTools {
    class kCGIException : exception {
    public:
        explicit kCGIException(int status) { this->status = status; }

        int status;
    };

    class kCgiData {
    public:
        kCgiData();
        kCgiData(unsigned char *data, size_t len,size_t offset = 0);
        void Put(unsigned char *data, size_t len,size_t offset = 0);

        ~kCgiData();

        void Print();
        void ToVector(vector<unsigned char> &data);

        unsigned char *Data = nullptr;
        size_t DataLen = 0;
        kCgiData *next = nullptr;
    };

    class kCGI {
    public:
        kCGI(const string &ip, int port) noexcept(false);

        explicit kCGI(const string &SockFilePath) noexcept(false);

        ~kCGI();

        int sendStartRequestRecord();

        int sendParams(const char *name, const char *value);

        int sendEndRequestRecord();

        size_t ReadFromPhp(map<string, string> &header, vector<unsigned char> &data);

    private:

        //生成头部
        FCGI_Header makeHeader(int type, int request,
                               int contentLength, int paddingLength);

        size_t Read(void *, size_t);

        //生成发起请求的请求体
        static FCGI_BeginRequestBody makeBeginRequestBody(int role, int keepConnection);

        //生成 PARAMS 的 name-value body
        static int makeNameValueBody(const char *name, int nameLen,
                                     const char *value, int valueLen,
                                     unsigned char *bodyBuffPtr, int *bodyLen);


        static char *findStartHtml(char *content);

        static void getHtmlFromContent(char *content);


    private:
        int SockFd = 0;
        int requestId = 0;
        static int RequestId;

        static const int PARAMS_BUFF_LEN = 0xFFFF;    //环境参数buffer的大小
        static const int CONTENT_BUFF_LEN = 0xFFFF;   //内容buffer的大小

    };

}


#endif //KHTTPD_KCGI_H
