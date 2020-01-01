//
// Created by caesar on 2019/12/2.
//

#ifndef KPROXYCPP_UTF8URL_H
#define KPROXYCPP_UTF8URL_H
/**
 *
 * int main()
 * {
 *     std::freopen("output", "w", stdout);
 *
 *     std::string url = u8"https://blog.csdn.net/FlushHip?type=1&name=老王&str={}%";
 *     std::cout << "before encode: " << url << std::endl;
 *     std::cout << "after encode: " << UTF8Url::Encode(url) << std::endl;
 *     std::cout << "after decode: " << UTF8Url::Decode(UTF8Url::Encode(url)) << std::endl;
 *     return 0;
 * }
 *
 */
#include <algorithm>
#include <string>
#include <stdexcept>

class UTF8Url {
public:
    static std::string Encode(const std::string &url);

    static std::string Decode(const std::string &url);

private:
    static const std::string &HEX_2_NUM_MAP();

    static const std::string &ASCII_EXCEPTION();

    static unsigned char NUM_2_HEX(char h, char l);
};


#endif //KPROXYCPP_UTF8URL_H
