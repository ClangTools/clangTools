#ifndef BMPTOOL_HEADER
#define BMPTOOL_HEADER

#include <stdio.h>
#include <stdlib.h>

namespace clangTools {
/**
     * BMP 文件头
     */
    typedef struct {
        unsigned short identity;  // 2 byte : "BM"則為BMP
        unsigned int file_size;   // 4 byte : 檔案size
        unsigned short reserved1; // 2 byte : 保留欄位,設為0
        unsigned short reserved2; // 2 byte : 保留欄位,設為0
        unsigned int data_offset; // 4 byte : RGB資料開始之前的資料偏移量
    } BITMAPFILEHEADER;

    const unsigned int SIZEOF_BITMAPFILEHEADER = 14;

/**
     * BMP 信息头
     */
    typedef struct {
        unsigned int header_size;      // 4 byte : struct BITMAPINFOHEADER的size
        int width;                     // 4 byte : 影像寬度(pixel)
        int height;                    // 4 byte : 影像高度(pixel)
        unsigned short planes;         // 2 byte : 設為1
        unsigned short bit_per_pixel;  // 2 byte : 每個pixel所需的位元數(1/4/8/16/24/32)
        unsigned int compression;      // 4 byte : 壓縮方式, 0 : 未壓縮
        unsigned int data_size;        // 4 byte : 影像大小,設為0
        int hresolution;               // 4 byte : pixel/m
        int vresolution;               // 4 byte : pixel/m
        unsigned int used_colors;      // 4 byte : 使用調色盤顏色數,0表使用調色盤所有顏色
        unsigned int important_colors; // 4 byte : 重要顏色數,當等於0或used_colors時,表全部都重要
    } BITMAPINFOHEADER;

/**
     * BMP 像素格式
     */
    typedef struct {
        unsigned char blue;     // 1 byte : 調色盤藍色
        unsigned char green;    // 1 byte : 調色盤綠色
        unsigned char red;      // 1 byte : 調色盤紅色
        unsigned char reserved; // 1 byte : 保留欄位,設為0
    } PALLETTE;

    class BMP {
    private:
        /**
         * 保存的文件头
         */
        BITMAPFILEHEADER Head;
        /**
         * 保存的信息头
         */
        BITMAPINFOHEADER Info;

        PALLETTE Quad[256] = {0};

        unsigned char *Data;

    public:
        static const char *TAG;

        /**
         * 初始化
         */
        BMP();

        BMP(const char *BmpPath);

        ~BMP();

        /**
         * 镜像
         *  type    镜像方向：
         *                   1 为左右
         *                   2 为上下
         */
        bool SetMirror(int type = 1);

        /**
         * 获取宽度
         */
        int GetWidth();

        BITMAPFILEHEADER *GetBITMAPFILEHEADER();

        BITMAPINFOHEADER *GetBITMAPINFOHEADER();

        PALLETTE *GetQuad();

        /**
         * 获取高度
         */
        int GetHeight();

        /**
         * 获取 位数
         */
        int GetBitPerPixel();

        /**
         * 获取数据大小
         */
        unsigned long GetDataSize();

        unsigned long GetStep();

        /**
         * 设置像素数据
         *  int width               图片宽度
         *  int height              图片高度
         *  int BitPerPixel         图片位数
         *  unsigned char * data    图片数据
         */
        bool SetData(int width, int height, int BitPerPixel, unsigned char *data);

        int split(BMP bmps[3]);

        /**
         * 获取像素数据
         */
        unsigned char *GetData();

        /**
         * 从文件读取
         */
        bool ReadBmp(const char *BmpPath);

        /**
         * 从流读取
         */
        bool ReadBmp(unsigned char *BmpPath);

        /**
         * 写入到文件
         */
        bool WriteBmp(const char *BmpPath);

        /**
         * 写入到流
         */
        int WriteBmp(unsigned char *BmpPath);

        /**
         * 将 BMP 头信息输出
         * @param bin
         * @param offset
         * @return
         */
        int WriteBmpInfo(unsigned char *bin, int offset = 0);
    };
} // namespace clangTools

#endif