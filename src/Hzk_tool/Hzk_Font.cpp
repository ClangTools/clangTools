//
// Created by caesar kekxv on 2020/4/5.
//

#include "Hzk_Font.h"

Hzk_Font Hzk_Font::hzkFont{};

int Hzk_Font::init(Hzk_Font::FontType fontType) {
    if (!ASC12.is_open())ASC12.init(1, 12, 6, "ASC12");
    if (!HZK12.is_open())HZK12.init(2, 12, 12, "HZK12");
    if (!ASC14.is_open())ASC14.init(1, 14, 7, "ASC14");
    if (!HZK14.is_open())HZK14.init(2, 14, 14, "HZK14");
    if (!ASC16.is_open())ASC16.init(1, 16, 8, "ASC16");
    if (!HZK16.is_open())HZK16.init(2, 16, 16, "HZK16");
    if (!ASC20.is_open())ASC20.init(2, 20, 10, "ASC20");
    if (!HZK20.is_open())HZK20.init(3, 20, 20, "HZK20");
    if (!ASC24.is_open())ASC24.init(2, 24, 12, "ASC24");
    if (!HZK24.is_open())HZK24.init(2, 24, 24, "HZK24");
    if (!ASC32.is_open())ASC32.init(2, 32, 16, "ASC32");
    if (!HZK32.is_open())HZK32.init(2, 32, 32, "HZK32");
    if (!ASC48.is_open())ASC48.init(3, 48, 24, "ASC48");
    if (!HZK48.is_open())HZK48.init(6, 48, 48, "HZK48");
    return 0;
}

Hzk_Font *Hzk_Font::instance() {
    hzkFont.init();
    return &hzkFont;
}

std::vector<cv::Mat> Hzk_Font::get(std::string str, Hzk_Font::FontSize fontSize) {
    std::vector<cv::Mat> outs;
    Hzk_tool *asc = nullptr;
    Hzk_tool *hzk = nullptr;
    switch (fontSize) {
        case Hzk_Font::FontSize::S12:
            asc = &ASC12;
            hzk = &HZK12;
            break;
        case Hzk_Font::FontSize::S14:
            asc = &ASC14;
            hzk = &HZK14;
            break;
        case Hzk_Font::FontSize::S16:
            asc = &ASC16;
            hzk = &HZK16;
            break;
        case Hzk_Font::FontSize::S20:
            asc = &ASC20;
            hzk = &HZK20;
            break;
        case Hzk_Font::FontSize::S24:
            asc = &ASC24;
            hzk = &HZK24;
            break;
        case Hzk_Font::FontSize::S32:
            asc = &ASC32;
            hzk = &HZK32;
            break;
        case Hzk_Font::FontSize::S48:
            asc = &ASC48;
            hzk = &HZK48;
            break;
        default:
            return outs;
    }


    unsigned int len = str.length() + 1;
    char *source = new char[len];
    std::strcpy(source, str.c_str());
    char *dest = new char[len];
    Hzk_tool::convertCode("GB2312", "utf-8", source, len, dest, len);
    delete[] source;

    unsigned int i = 0;
    while (i < strlen(dest)) {
        byte c = dest[i];
        if (c < 0x7f) {
            cv::Mat mat = asc->getOne(&dest[i]);
            outs.push_back(mat);
        } else if (c >= 0xa1) {
            cv::Mat mat = hzk->getOne(&dest[i]);
            outs.push_back(mat);
            i++;
        }
        i++;
    }
    delete[] dest;

    return outs;
}
