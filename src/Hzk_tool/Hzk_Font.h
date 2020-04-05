//
// Created by caesar kekxv on 2020/4/5.
//

#ifndef RASPICLOCKOLED_HZK_FONT_H
#define RASPICLOCKOLED_HZK_FONT_H

#include <Hzk_tool.h>

class Hzk_Font {
public:
    static Hzk_Font *instance();

    enum FontSize {
        S12,
        S14,
        S16,
        S20,
        S24,
        S32,
        S48,
    };
    enum FontType {
        None,
    };

public:
    std::vector<cv::Mat> get(std::string str, FontSize fontSize = S14);

private:
    static Hzk_Font hzkFont;

private:
    int init(FontType fontType = None);

    Hzk_tool ASC12, ASC14, ASC16, ASC20,ASC24, ASC32, ASC48;
    Hzk_tool HZK12, HZK14, HZK16, HZK20,HZK24, HZK32, HZK48;
};


#endif //RASPICLOCKOLED_HZK_FONT_H
