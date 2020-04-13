//
// Created by caesar kekxv on 2020/4/11.
//
#if 0
#include <iostream>
#include <string>
#include <locale>
#include <codecvt>
#include <fstream>

int main(int argc, char *argv[]) {
    std::wstring str = L"123,我是谁？我爱钓鱼岛！";

    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

    std::string narrowStr = conv.to_bytes(str);
    {
        std::ofstream ofs("c:\\test.txt");
        ofs << narrowStr;
    }

    std::wstring wideStr = conv.from_bytes(narrowStr);
    {
        std::locale::global(std::locale("Chinese-simplified"));
        std::wofstream ofs(L"c:\\testW.txt");
        ofs << wideStr;
    }
}
#endif

#include <string>
#include <iconv.h>
#include <cstring>
#include <logger.h>

using namespace std;

int code_convert(char *from_charset, char *to_charset,
                 char *inBuff, size_t inlen, char *outbuf, size_t outlen) {
    iconv_t cd;
    char **pin = &inBuff;
    char **pout = &outbuf;
    cd = iconv_open(to_charset, from_charset);
    if (cd == (iconv_t) -1) {
        return -1;
    }

    memset(outbuf, 0, outlen);
    if (iconv(cd, pin, &inlen, pout, &outlen) == -1) {
        logger::instance()->e(__FILENAME__, __LINE__, "errno=%d", errno);
        iconv_close(cd);
        return -1;
    }
    iconv_close(cd);
    return 0;
}

int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
#ifdef WIN32
    //获取所需缓冲区大小
    int nUtf8Count = WideCharToMultiByte(CP_ACP, 0, (const wchar_t *) inbuf, inlen /2, nullptr, 0, nullptr, nullptr);
    if (nUtf8Count == 0) {
        return -1;
    }

    //此处理解：wide char是windows对UTF16的存储实现，
    //传递CP_UTF8是告诉【输出】的字节流为UTF8格式，
    //这样函数内部就知道将UTF16的wide char转化成什么格式的字节流了
    char *pUtf8Buff = new char[nUtf8Count];
    WideCharToMultiByte(CP_ACP, 0, (const wchar_t *) inbuf, inlen /2, pUtf8Buff, nUtf8Count, nullptr, nullptr);
    memcpy(outbuf,pUtf8Buff,nUtf8Count);
    outbuf[nUtf8Count] = 0;
    delete[]pUtf8Buff;
    return 0;
#else
    return code_convert((char *) "UTF-16LE", (char *) "UTF-8//TRANSLIT", inbuf, inlen, outbuf, outlen);
#endif
}

string g2u(const wstring &str) {
    string outstr;
    char *_tmp = new char[str.length() * 2 + 100];
    g2u((char *) str.data(), str.length() * 2,
        _tmp, str.length() * 2 + 100);
    outstr = _tmp;
    delete[]_tmp;
    return outstr;
}

int main(int argc, char *argv[]) {
    logger::instance()->init_default();
    std::wstring str = L"测试";
    logger::instance()->i(__FILENAME__, __LINE__, "%ls", str.c_str());
    logger::instance()->i(__FILENAME__, __LINE__, "%s", g2u(str).c_str());

    return 0;
}