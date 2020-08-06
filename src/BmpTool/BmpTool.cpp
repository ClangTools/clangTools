#include "BmpTool.h"
#include <logger.h>

namespace clangTools {

#ifdef __FILENAME__
    const char *BMP::TAG = __FILENAME__;
#else
    const char *BMP::TAG = "BMP";
#endif

/**
 * 初始化
 */
    BMP::BMP() {
        memset(&Head, 0x00, sizeof(Head));
        memset(&Info, 0x00, sizeof(Info));
        Data = nullptr;
    }

/**
 * 初始化 载入默认照片
 *   const char *BmpPath  需要读取的照片
 */
    BMP::BMP(const char *BmpPath) {
        memset(&Head, 0x00, sizeof(Head));
        memset(&Info, 0x00, sizeof(Info));
        Data = nullptr;

        ReadBmp(BmpPath);
    }

/**
 *
 */
    BMP::~BMP() {
        if (Data != nullptr) {
            free(Data);
            Data = nullptr;
        }
    }

/**
 * 获取宽度
 */
    int BMP::GetWidth() { return Info.width > 0 ? Info.width : -Info.width; }

    BITMAPFILEHEADER *BMP::GetBITMAPFILEHEADER() {
        return &Head;
    }

    BITMAPINFOHEADER *BMP::GetBITMAPINFOHEADER() {
        return &Info;
    }

    PALLETTE *BMP::GetQuad() {
        return (PALLETTE *) Quad;
    }

/**
 * 获取高度
 */
    int BMP::GetHeight() { return Info.height > 0 ? Info.height : -Info.height; }

/**
 * 获取 位数
 * @return
 */
    int BMP::GetBitPerPixel() { return Info.bit_per_pixel; }

/**
 * 获取大小
 * @return
 */
    unsigned long BMP::GetDataSize() { return (((GetWidth() * (GetBitPerPixel() / 8) + 3) / 4) * 4) * GetHeight(); }

    unsigned long BMP::GetStep() { return ((GetWidth() * (GetBitPerPixel() / 8) + 3) / 4) * 4; }

/**
 * 镜像
 *  type    镜像方向：
 *                   1 为左右
 *                   2 为上下
 */
    bool BMP::SetMirror(int type) {
        int width = GetWidth();
        int height = GetHeight();
        unsigned char *lrgb = Data;
        unsigned char *colors = nullptr;
        unsigned long size = ((width * GetBitPerPixel() / 8 + 3) / 4 * 4 * height);
        colors = (unsigned char *) malloc(size);
        if (colors == nullptr) {
            logger::instance()->d(TAG, __LINE__, "开辟内存失败.");
            return false;
        }
        int _index = (GetBitPerPixel() / 8);
        int flag = GetBitPerPixel() == 32 ? 1 : 0;
        switch (type) {
            case 2: {
                int w = (width * GetBitPerPixel() / 8 + 3) / 4 * 4;
                for (int h = 0; h < height; h++) {
                    memcpy(&colors[h * w], &lrgb[(height - h - 1) * w], static_cast<size_t>(w));
                }
            }
                break;
            case 1:
            default: {
                for (int i = 0; i < width * height; i++) {
                    int _h = i / width;
                    int _w = i % width;

                    int index = width * _h * _index + _w * _index;
                    int index1 = width * (_h + 1) * _index - (_w - 1) * _index;
                    for (int _i = 0; _i < _index; _i++)
                        colors[index + _i] = lrgb[index1 + _i];
                }
            }
                break;
        }
        free(Data);
        Data = colors;
        return true;
    }

/**
 * 获取像素数据
 */
    unsigned char *BMP::GetData() {
        return Data;
    }

/**
 * 设置像素数据
 *  int width               图片宽度
 *  int height              图片高度
 *  int BitPerPixel         图片位数
 *  unsigned char * data    图片数据
 */
    bool BMP::SetData(int width, int height, int BitPerPixel, unsigned char *data) {
        Head.identity = 0x4d42;
        Head.file_size = (width * height * BitPerPixel / 8) + SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER);
        Head.reserved1 = 0;
        Head.reserved2 = 0;
        Head.data_offset = SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) - sizeof(Head.identity);

        Info.header_size = sizeof(BITMAPINFOHEADER);
        Info.width = width;
        Info.height = height;
        Info.planes = 1;
        Info.bit_per_pixel = static_cast<unsigned short>(BitPerPixel);
        Info.compression = 0;
        Info.data_size = 0;
        Info.hresolution = 0;
        Info.vresolution = 0;
        Info.used_colors = 0;
        Info.important_colors = 0;

        if (Data != nullptr) {
            free(Data);
            Data = nullptr;
        }
        int size = ((width * BitPerPixel / 8 + 3) / 4 * 4 * height);
        Data = (unsigned char *) malloc(size * sizeof(unsigned char));
        if (Data == nullptr) {
            logger::instance()->d(TAG, __LINE__, "开辟内存失败.");
            return false;
        }
        memcpy(Data, data, size * sizeof(unsigned char));
        return true;
    }

/**
 * 从文件读取
 */
    bool BMP::ReadBmp(const char *BmpPath) {
        FILE *fp_bmp = nullptr;
        fp_bmp = fopen(BmpPath, "rb");
        if (fp_bmp == nullptr) {
            logger::instance()->d(TAG, __LINE__, "文件打开失败.");
            return false;
        }
        if (fread(&Head.identity, sizeof(Head.identity), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP文件头失败.");
            return false;
        }
        if (Head.identity != 0x4d42) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP文件头失败.");
            return false;
        }
        if (fread(&Head.file_size, sizeof(Head.file_size), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP文件头失败.");
            return false;
        }
        if (fread(&Head.reserved1, sizeof(Head.reserved1), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP文件头失败.");
            return false;
        }
        if (fread(&Head.reserved2, sizeof(Head.reserved2), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP文件头失败.");
            return false;
        }
        if (fread(&Head.data_offset, sizeof(Head.data_offset), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP文件头失败.");
            return false;
        }

        if (fread(&Info.header_size, sizeof(Info.header_size), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.width, sizeof(Info.width), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.height, sizeof(Info.height), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.planes, sizeof(Info.planes), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.bit_per_pixel, sizeof(Info.bit_per_pixel), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.compression, sizeof(Info.compression), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.data_size, sizeof(Info.data_size), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.hresolution, sizeof(Info.hresolution), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.vresolution, sizeof(Info.vresolution), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.used_colors, sizeof(Info.used_colors), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }
        if (fread(&Info.important_colors, sizeof(Info.important_colors), 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "读取BMP信息头失败.");
            return false;
        }

        if (Data != NULL) {
            free(Data);
            Data = NULL;
        }
        unsigned long size = GetDataSize();
        Data = (unsigned char *) malloc(size);
        if (Data == NULL) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "开辟内存失败.");
            return false;
        }
        if (fread(Data, sizeof(unsigned char), size, fp_bmp) < size) {
            fclose(fp_bmp);
            free(Data);
            Data = NULL;
            logger::instance()->d(TAG, __LINE__, "像素读取失败.");
            return false;
        }
        fclose(fp_bmp);
        return true;
    }

/**
 * 从文件读取
 */
    bool BMP::ReadBmp(unsigned char *BmpPath) {
        int offset = 0;
        memcpy(&Head.identity, &BmpPath[offset], sizeof(Head.identity));
        offset += sizeof(Head.identity);

        if (Head.identity != 0x4d42) {
            logger::instance()->d(TAG, __LINE__, "读取BMP文件头失败.");
            return false;
        }
        memcpy(&Head.file_size, &BmpPath[offset], sizeof(Head.file_size));
        offset += sizeof(Head.file_size);

        memcpy(&Head.reserved1, &BmpPath[offset], sizeof(Head.reserved1));
        offset += sizeof(Head.reserved1);

        memcpy(&Head.reserved2, &BmpPath[offset], sizeof(Head.reserved2));
        offset += sizeof(Head.reserved2);

        memcpy(&Head.data_offset, &BmpPath[offset], sizeof(Head.data_offset));
        offset += sizeof(Head.data_offset);

        memcpy(&Info.header_size, &BmpPath[offset], sizeof(Info.header_size));
        offset += sizeof(Info.header_size);

        memcpy(&Info.width, &BmpPath[offset], sizeof(Info.width));
        offset += sizeof(Info.width);

        memcpy(&Info.height, &BmpPath[offset], sizeof(Info.height));
        offset += sizeof(Info.height);

        memcpy(&Info.planes, &BmpPath[offset], sizeof(Info.planes));
        offset += sizeof(Info.planes);

        memcpy(&Info.bit_per_pixel, &BmpPath[offset], sizeof(Info.bit_per_pixel));
        offset += sizeof(Info.bit_per_pixel);

        memcpy(&Info.compression, &BmpPath[offset], sizeof(Info.compression));
        offset += sizeof(Info.compression);

        memcpy(&Info.data_size, &BmpPath[offset], sizeof(Info.data_size));
        offset += sizeof(Info.data_size);

        memcpy(&Info.hresolution, &BmpPath[offset], sizeof(Info.hresolution));
        offset += sizeof(Info.hresolution);

        memcpy(&Info.vresolution, &BmpPath[offset], sizeof(Info.vresolution));
        offset += sizeof(Info.vresolution);

        memcpy(&Info.used_colors, &BmpPath[offset], sizeof(Info.used_colors));
        offset += sizeof(Info.used_colors);

        memcpy(&Info.important_colors, &BmpPath[offset], sizeof(Info.important_colors));
        offset += sizeof(Info.important_colors);

        if (Data != NULL) {
            free(Data);
            Data = NULL;
        }
        unsigned long size = GetDataSize();
        Data = (unsigned char *) malloc(size);
        if (Data == NULL) {
            logger::instance()->d(TAG, __LINE__, "开辟内存失败.");
            return false;
        }

        memcpy(Data, &BmpPath[offset], size);
        offset += size;
        return true;
    }

/**
 * 写入到文件
 */
    bool BMP::WriteBmp(const char *BmpPath) {
        if (Data == NULL) {
            return false;
        }
        FILE *fp_bmp = NULL;
        fp_bmp = fopen(BmpPath, "wb");
        if (fp_bmp == NULL) {
            logger::instance()->d(TAG, __LINE__, "文件打开失败.");
            return false;
        }

        unsigned char _bmpHeadinfo[54]{};
        WriteBmpInfo(_bmpHeadinfo);
        if (fwrite(_bmpHeadinfo, 54, 1, fp_bmp) < 1) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "写入BMP文件头失败.");
            return false;
        }

        if (GetBitPerPixel() == 8) {
            if (fwrite((unsigned char *) &Quad, sizeof(PALLETTE) * 256, 1, fp_bmp) < 1) {
                fclose(fp_bmp);
                logger::instance()->d(TAG, __LINE__, "写入BMP信息Quad失败.");
                return false;
            }
        }

        unsigned long size = GetDataSize();
        if (fwrite(Data, sizeof(unsigned char), size, fp_bmp) < size) {
            fclose(fp_bmp);
            logger::instance()->d(TAG, __LINE__, "像素写入失败.");
            return false;
        }
        fclose(fp_bmp);
        return true;
    }

/**
 * 写入到文件
 */
    int BMP::WriteBmp(unsigned char *BmpPath) {
        if (Data == NULL) {
            return false;
        }
        int offset = WriteBmpInfo(BmpPath);
        if (GetBitPerPixel() == 8) {
            memcpy(&BmpPath[offset], (unsigned char *) &Quad, sizeof(PALLETTE) * 256);
            offset += sizeof(PALLETTE) * 256;
        }

        unsigned long size = GetDataSize();

        memcpy(&BmpPath[offset], Data, size);
        // offset += size;
        return offset += size;
    }

    int BMP::split(BMP bmps[3]) {
        if (GetBitPerPixel() != 24 && GetBitPerPixel() != 32) {
            logger::instance()->w(__FILENAME__, __LINE__, "只支持 24位 以及 32位 分割，当前 ： %d", GetBitPerPixel());
            return -1;
        }

        int W = GetWidth();
        int H = GetHeight();

        PALLETTE quad[256];
        BITMAPFILEHEADER bfh;
        BITMAPINFOHEADER bih;
        //设定BITMAPFILEHEADER
        memset(&bfh, 0, sizeof(bfh));
        bfh.identity = 0x4d42;
        bfh.file_size = SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(PALLETTE) * 256 +
                        (((W * (8 / 8) + 3) / 4) * 4) * H;
        bfh.data_offset = SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(PALLETTE) * 256;

        //设定BITMAPINFOHEADER
        memset(&bih, 0, sizeof(bih));
        bih.header_size = sizeof(bih);
        bih.width = W;
        bih.height = H;
        bih.planes = 1;   //必须为1
        bih.bit_per_pixel = 8;
        bih.compression = 0; //BI_RGB

        //设定调色盘数据 quad
        for (int n = 0; n < 256; n++) {
            quad[n].blue = n;
            quad[n].green = n;
            quad[n].red = n;
            quad[n].reserved = 0;
        }

        for (int i = 0; i < 3; i++) {
            memcpy(&(bmps[i].Head), (unsigned char *) &bfh, SIZEOF_BITMAPFILEHEADER);
            memcpy(&(bmps[i].Info), (unsigned char *) &bih, sizeof(BITMAPINFOHEADER));
            memcpy(&(bmps[i].Quad), (unsigned char *) &quad, sizeof(PALLETTE) * 256);

            if (bmps[i].Data != nullptr) {
                free(bmps[i].Data);
                bmps[i].Data = nullptr;
            }
            int size = bmps[i].GetDataSize();
            bmps[i].Data = (unsigned char *) malloc(size * sizeof(unsigned char));
            if (bmps[i].Data == nullptr) {
                logger::instance()->d(__FILENAME__, __LINE__, "开辟内存失败.");
                return -1;
            }
        }
        int step = GetStep();
        int mstep = bmps[0].GetStep();
        for (int h = 0; h < GetHeight(); h++) {
            for (int w = 0; w < GetWidth(); w++) {
                int index = h * step + w * GetBitPerPixel() / 8;
                int mindex = h * mstep + w;
                for (int i = 0; i < 3; i++) {
                    bmps[i].Data[mindex] = Data[index + i];
                }
            }
        }
        return 0;
    }

    int BMP::WriteBmpInfo(unsigned char *bin, int offset) {
        memcpy(&bin[offset], &Head.identity, sizeof(Head.identity));
        offset += sizeof(Head.identity);

        memcpy(&bin[offset], &Head.file_size, sizeof(Head.file_size));
        offset += sizeof(Head.file_size);

        memcpy(&bin[offset], &Head.reserved1, sizeof(Head.reserved1));
        offset += sizeof(Head.reserved1);

        memcpy(&bin[offset], &Head.reserved2, sizeof(Head.reserved2));
        offset += sizeof(Head.reserved2);

        memcpy(&bin[offset], &Head.data_offset, sizeof(Head.data_offset));
        offset += sizeof(Head.data_offset);

        memcpy(&bin[offset], &Info.header_size, sizeof(Info.header_size));
        offset += sizeof(Info.header_size);

        memcpy(&bin[offset], &Info.width, sizeof(Info.width));
        offset += sizeof(Info.width);

        memcpy(&bin[offset], &Info.height, sizeof(Info.height));
        offset += sizeof(Info.height);

        memcpy(&bin[offset], &Info.planes, sizeof(Info.planes));
        offset += sizeof(Info.planes);

        memcpy(&bin[offset], &Info.bit_per_pixel, sizeof(Info.bit_per_pixel));
        offset += sizeof(Info.bit_per_pixel);

        memcpy(&bin[offset], &Info.compression, sizeof(Info.compression));
        offset += sizeof(Info.compression);

        memcpy(&bin[offset], &Info.data_size, sizeof(Info.data_size));
        offset += sizeof(Info.data_size);

        memcpy(&bin[offset], &Info.hresolution, sizeof(Info.hresolution));
        offset += sizeof(Info.hresolution);

        memcpy(&bin[offset], &Info.vresolution, sizeof(Info.vresolution));
        offset += sizeof(Info.vresolution);

        memcpy(&bin[offset], &Info.used_colors, sizeof(Info.used_colors));
        offset += sizeof(Info.used_colors);

        memcpy(&bin[offset], &Info.important_colors, sizeof(Info.important_colors));
        offset += sizeof(Info.important_colors);
        return offset;
    }

} // namespace clangTools