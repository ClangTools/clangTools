//
// Created by caesar kekxv on 2020/4/4.
//

#ifndef RASPICLOCKOLED_HZK_TOOL_H
#define RASPICLOCKOLED_HZK_TOOL_H


#include <cstdio>
#include <fstream>
#include <block.h>

#ifdef ENABLE_OPENCV

#include <opencv2/opencv.hpp>

#endif

class Hzk_tool {
public:
    Hzk_tool();

    ~Hzk_tool();

    bool init(int _byte_in_row = 1, int _row = 12, int _cols = 8, const char *font_path = "ASC12");

    bool get(std::vector<std::vector<std::vector<unsigned char>>> &, const std::string &str);

    inline bool is_open() { return fin.is_open(); }

#ifdef ENABLE_OPENCV

    cv::Mat getOne(char *dest);
    std::vector<cv::Mat> get(const std::string &str);

#endif

public:
    static int
    convertCode(const char *tocode, const char *fromcode, char *inbuff, size_t inlen, char *outbuff, size_t outlen);

private:
    std::ifstream fin;
    bool is_dword = false;
    int byte_in_row = 2;
    int row_count = 12;
    int col_count = 8;
    int move_step = 0x00;
    byte move_direction = 0x00;
    int length = byte_in_row * row_count;

    char *p = nullptr;//new char[length];
    Block *block = nullptr;//(p, length, byte_in_row);
};


#endif //RASPICLOCKOLED_HZK_TOOL_H
