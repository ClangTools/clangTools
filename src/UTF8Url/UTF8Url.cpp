//
// Created by caesar on 2019/12/2.
//

#include "UTF8Url.h"

const std::string & UTF8Url::HEX_2_NUM_MAP()
{
    static const std::string str("0123456789ABCDEF");
    return str;
}

const std::string & UTF8Url::ASCII_EXCEPTION()
{
    static const std::string str(R"("%<>[\]^_`{|})");
    return str;
}

unsigned char UTF8Url::NUM_2_HEX(const char h, const char l)
{
    unsigned char hh = std::find(std::begin(HEX_2_NUM_MAP()), std::end(HEX_2_NUM_MAP()), h) - std::begin(HEX_2_NUM_MAP());
    unsigned char ll = std::find(std::begin(HEX_2_NUM_MAP()), std::end(HEX_2_NUM_MAP()), l) - std::begin(HEX_2_NUM_MAP());
    return (hh << 4) + ll;
}

std::string UTF8Url::Encode(const std::string & url)
{
    std::string ret;
    for (char it : url)
    {
        if (((it >> 7) & 1) || (std::count(std::begin(ASCII_EXCEPTION()), std::end(ASCII_EXCEPTION()), it)))
        {
            ret.push_back('%');
            ret.push_back(HEX_2_NUM_MAP()[(it >> 4) & 0x0F]);
            ret.push_back(HEX_2_NUM_MAP()[it & 0x0F]);
        }
        else
        {
            ret.push_back(it);
        }
    }
    return ret;
}

std::string UTF8Url::Decode(const std::string & url)
{
    std::string ret;
    for (auto it = url.begin(); it != url.end(); ++it)
    {
        if (*it == '%')
        {
            if (std::next(it++) == url.end())
            {
                throw std::invalid_argument("url is invalid");
            }
            ret.push_back(NUM_2_HEX(*it, *std::next(it)));
            if (std::next(it++) == url.end())
            {
                throw std::invalid_argument("url is invalid");
            }
        }
        else
        {
            ret.push_back(*it);
        }
    }
    return ret;
}