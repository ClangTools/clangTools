//
// Created by caesar kekxv on 2020/4/4.
//

#include <iconv.h>
#include <cstring>
#include <logger.h>
#include "Hzk_tool.h"

Hzk_tool::Hzk_tool() = default;


int Hzk_tool::convertCode(const char *tocode, const char *fromcode, char *inbuff, size_t inlen, char *outbuff,
                          size_t outlen) {
    iconv_t cd = iconv_open(tocode, fromcode);
    memset(outbuff, 0, outlen);
    if (cd == nullptr || iconv(cd, &inbuff, &inlen, &outbuff, &outlen) == (size_t) (-1))
        return -1;

    iconv_close(cd);
    return 0;
}

bool Hzk_tool::init(int _byte_in_row, int _row, int _cols, const char *font_path) {
    if (fin.is_open())fin.close();
    byte_in_row = _byte_in_row;
    row_count = _row;
    col_count = _cols;
    length = byte_in_row * row_count;
    is_dword = std::string(font_path).find("ASC") == std::string::npos;
    auto path = (logger::get_local_path() + logger::path_split + "font" + logger::path_split + font_path);
    fin.open(path, std::ios::binary);

    if (p) {
        delete[]p;
        p = nullptr;
    }
    if (block) {
        delete block;
        block = nullptr;
    }
    p = new char[length];
    block = new Block(p, length, byte_in_row, col_count);

    return fin.is_open();
}

bool Hzk_tool::get(std::vector<std::vector<std::vector<unsigned char>>> &out, const std::string &str) {
    if (!fin.is_open())return false;
    unsigned int len = str.length() + 1;
    char *source = new char[len];
    std::strcpy(source, str.c_str());
    char *dest = new char[len];
    convertCode("GB2312", "utf-8", source, len, dest, len);
    delete[] source;

    unsigned int i = 0;
    while (i < strlen(dest)) {
        byte c = dest[i];
        if (c < 0x7f && !is_dword) {
            fin.seekg(length * dest[i]);
            fin.read(p, length);
            // printFont(block, var_in_row, show_pattern, transform, move_direction, move_step);
            // std::cout << block->getPatternString() << std::endl;
            out.push_back(block->getPattern());
            // flag = true;
        } else if (c >= 0xa1 && is_dword) {
            fin.seekg(((c - 0xa1) * 94 + (byte) dest[i + 1] - 0xa1) * length);
            fin.read(p, length);
            // printFont(block, var_in_row, show_pattern, transform, move_direction, move_step);
            // std::cout << block->getPatternString() << std::endl;
            out.push_back(block->getPattern());
            i++;
            // flag = true;
        } else {
            logger::instance()->e(TAG, __LINE__, "No pattern for %c", (char) dest[i]);
        }
        i++;
    }


    delete[] dest;
    return !out.empty();
}

#ifdef ENABLE_OPENCV

cv::Mat Hzk_tool::getOne(char *dest) {
    cv::Mat out;
    if (!fin.is_open())return out;
    byte c = dest[0];
    std::vector<std::vector<unsigned char>> item;
    if (c < 0x7f && !is_dword) {
        fin.seekg(length * dest[0]);
        fin.read(p, length);
        item = (block->getPattern());
    } else if (c >= 0xa1 && is_dword) {
        fin.seekg(((c - 0xa1) * 94 + (byte) dest[1] - 0xa1) * length);
        fin.read(p, length);
        item = (block->getPattern());
    } else {
        logger::instance()->e(TAG, __LINE__, "No pattern for %c", (char) dest[0]);
        return out;
    }
    if (item.empty())return out;
    cv::Mat tmp(item.size(), item[0].size(), CV_8UC3, cv::Scalar(255, 255, 255)); // create a black background
    for (int y = 0; y < item.size(); y++) {
        for (int x = 0; x < item[y].size(); x++) {
            unsigned char scalar = item[y][x] == 0 ? 255 : 0;
            tmp.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(scalar, scalar, scalar);
        }
    }
    return tmp;
}

std::vector<cv::Mat> Hzk_tool::get(const std::string &str) {
    std::vector<cv::Mat> inMats;
    std::vector<std::vector<std::vector<unsigned char>>> out;
    get(out, str);
    for (auto &item:out) {
        if (item.empty())continue;
        cv::Mat tmp(item.size(), item[0].size(), CV_8UC3, cv::Scalar(255, 255, 255)); // create a black background
        for (int y = 0; y < item.size(); y++) {
            for (int x = 0; x < item[y].size(); x++) {
                unsigned char scalar = item[y][x] == 0 ? 255 : 0;
                tmp.at<cv::Vec3b>(cv::Point(x, y)) = cv::Vec3b(scalar, scalar, scalar);
            }
        }
        inMats.push_back(tmp);
    }
    return inMats;
}

#endif

Hzk_tool::~Hzk_tool() {
    if (fin.is_open())fin.close();
    if (p) {
        delete[]p;
        p = nullptr;
    }
    if (block) {
        delete block;
        block = nullptr;
    }
}

