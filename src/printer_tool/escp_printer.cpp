//
// Created by caesar kekxv on 2020/8/5.
//

#include "escp_printer.h"
#include <string>
#include <logger.h>

#ifdef ENABLE_OPENCV

#include <opencv2/opencv.hpp>

#endif

#include <BmpTool.h>

using namespace clangTools;
using namespace std;
using namespace cv;


#ifdef __FILENAME__
const char *escp_printer::TAG = __FILENAME__;
#else
const char * escp_printer::TAG = "escp_printer";
#endif
escp_printer::escp_printer(PrinterToolReadWriteAgent *agent) {
    setPrinterToolReadWriteAgent(agent);
}

std::string escp_printer::PrinterStatus(const std::string &Status) {
    string message = "Unknown";

    if (Status == "00000") { message = "Idling"; }
    else if (Status == "00001") { message = "Printing"; }
    else if (Status == "00500") { message = "Head cooling down"; }
    else if (Status == "00510") { message = "Cooling the paper winding motor"; }
    else if (Status == "00900") { message = "Standby Mode"; }

    else if (Status == "01000") { message = "Cover is open"; }
    else if (Status == "01010") { message = "No Scrap box"; }
    else if (Status == "01100") { message = "Paper End"; }
    else if (Status == "01200") { message = "Ribbon End"; }
    else if (Status == "01300") { message = "Paper jam"; }
    else if (Status == "01400") { message = "Ribbon errors (detect error, ribbon break)"; }
    else if (Status == "01500") { message = "Paper Definition Error (The setting is different from printer setting)"; }
    else if (Status == "01600") { message = "Data error (improper data) "; }

    else if (Status == "02000") { message = "Head voltage error"; }
    else if (Status == "02100") { message = "Head position error"; }
    else if (Status == "02200") { message = "Power supply fun stopped  "; }
    else if (Status == "02300") { message = "Cutter error (Cut jamming etc.) "; }
    else if (Status == "02400") { message = "Pinch roller position error  "; }
    else if (Status == "02500") { message = "Abnormal head temperature  "; }
    else if (Status == "02600") { message = "Abnormal media temperature  "; }
    else if (Status == "02610") { message = "Abnormal temperature of paper winding motor  "; }
    else if (Status == "02700") { message = "Ribbon tension error  "; }
    else if (Status == "02800") { message = "RF-ID module error  "; }

    else if (Status == "03000") { message = "System error  "; }

    return message;
}

void escp_printer::setPrinterToolReadWriteAgent(PrinterToolReadWriteAgent *agent) {
    if (agent == nullptr)return;
    printerToolReadWriteAgent = agent;
}

bool escp_printer::isAgentReady() {
    return printerToolReadWriteAgent != nullptr;
}

void escp_printer::initSendData(unsigned char *data, const char *model) {
    memset(data, ' ', 32);
    data[0] = 0x1B;
    data[1] = 'P';
    memcpy(&data[2], model, strlen(model));
}

bool escp_printer::ResetPrinter() {
    if (!isAgentReady())return false;
    unsigned char data[32] = {0};
    memset(data, ' ', 32);
    data[0] = 0x1B;
    data[1] = '@';
    if (32 == this->printerToolReadWriteAgent->Send(data, 32)) {
        // read(data, 32);
        return true;
    }
    logger::instance()->e(TAG, __LINE__, "ResetPrinter is fail");
    return false;
}

int escp_printer::Get(std::string &OutData, escp_printer::ModelType model, bool readFlag) {
    if (!isAgentReady())return Error_AgentNotReady;
    string mode;
    string code;
    switch (model) {
        case escp_printer::ModelType::STATUS :
            mode = "STATUS";
            code = "";
            break;
        case escp_printer::ModelType::INFO_FVER :
            mode = "INFO";
            code = "FVER";
            break;
        case escp_printer::ModelType::INFO_MEDIA :
            mode = "INFO";
            code = "MEDIA";
            break;
        case escp_printer::ModelType::INFO_RESOLUTION_H :
            mode = "INFO";
            code = "RESOLUTION_H";
            break;
        case escp_printer::ModelType::INFO_RESOLUTION_V :
            mode = "INFO";
            code = "RESOLUTION_V";
            break;
        case escp_printer::ModelType::INFO_FREE_PBUFFER :
            mode = "INFO";
            code = "FREE_PBUFFER";
            break;
        case escp_printer::ModelType::INFO_MQTY_DEFAULT :
            mode = "INFO";
            code = "MQTY_DEFAULT";
            break;
        case escp_printer::ModelType::INFO_MQTY :
            mode = "INFO";
            code = "MQTY";
            break;
        case escp_printer::ModelType::INFO_RQTY :
            mode = "INFO";
            code = "RQTY";
            logger::instance()->w(TAG, __LINE__, "do not get INFO RQTY");
            return Error_ParamFail;
        case escp_printer::ModelType::INFO_SERIAL_NUMBER :
            mode = "INFO";
            code = "SERIAL_NUMBER";
            break;
        default:
            logger::instance()->e(TAG, __LINE__, "Error_ParamFail");
            return Error_ParamFail;
    }
    if (isAgentReady()) {

        unsigned char data[32] = {0};
        initSendData(data, mode.c_str());
        memcpy(&data[8], code.c_str(), code.size());

        int sRet = printerToolReadWriteAgent->Send(data, 32), rRet = 0, rRet1 = 0;
        if (sRet == 32) {
            if (readFlag) {
                unsigned char rData[64], rData1[64];
                memset(rData, 0x00, 64);
                memset(rData1, 0x00, 64);
                rRet = printerToolReadWriteAgent->Read(rData, 32);
                if (rRet < 0) {
                    return Error_FAIL;
                }
                rRet1 = printerToolReadWriteAgent->Read(rData1, 32);
                if (rRet1 > 0) {
                    rRet = rRet1;
                    memcpy(rData, rData1, 32);
                }
                if (rRet > 0) {

                    char Data[65] = {0};
                    memcpy(Data, rData, rRet);
                    Data[rRet] = 0;
                    unsigned char *p = (unsigned char *) memchr(Data, '\r', rRet);
                    if (p != nullptr) {
                        *p = 0;
                    }
                    OutData = Data;
                    sRet = OutData.size();
                }
            }
        }
        return sRet;

    }
    return Error_FAIL;
}

std::string escp_printer::Get(escp_printer::ModelType model) {
    string data;
    Get(data, model, true);
    return data;
}

bool escp_printer::PutImage(ImageType type, int len) {
    if (!isAgentReady())return Error_AgentNotReady;
    string code = "";
    switch (type) {
        case ImageType::YPLANE:
            code = "YPLANE";
            break;
        case ImageType::MPLANE:
            code = "MPLANE";
            break;
        case ImageType::CPLANE:
            code = "CPLANE";
            break;
        case ImageType::MULTICUT:
            code = "MULTICUT";
            break;
        default:
            logger::instance()->e(TAG, __LINE__, "Error_ParamFail");
            return Error_ParamFail;
    }
    unsigned char data[32] = {0};
    char Argument3[9] = {0};
    initSendData(data, "IMAGE");
    memcpy(&data[8], code.c_str(), code.size());
    sprintf(Argument3, "%08d", len);
    memcpy(&data[24], Argument3, 8);

    int sRet = printerToolReadWriteAgent->Send(data, 32), rRet = 0, rRet1 = 0;
    return (sRet == 32);
}

int escp_printer::CNTRL(escp_printer::CNTRLType type, unsigned char *sData, int len) {
    if (!isAgentReady())return Error_AgentNotReady;

    unsigned char data[32] = {0};
    char Argument3[9] = {0};
    initSendData(data, "CNTRL");
    memset(&data[8], 0x20, 16);
    string code;
    switch (type) {
        case escp_printer::CNTRLType::START:
            code = "START";
            break;
        case escp_printer::CNTRLType::CUTTER:
            code = "CUTTER";
            break;
        case escp_printer::CNTRLType::OVERCOAT:
            code = "OVERCOAT";
            break;
        case escp_printer::CNTRLType::BUFFCNTRL:
            code = "BUFFCNTRL";
            break;
        case escp_printer::CNTRLType::DUPLEX_CANCEL:
            code = "DUPLEX_CANCEL";
            break;
        case escp_printer::CNTRLType::FULL_CUTTER_SET:
            code = "FULL_CUTTER_SET";
            break;
        default:
            logger::instance()->e(TAG, __LINE__, "Error_ParamFail");
            return Error_ParamFail;
    }
    memcpy(&data[8], code.c_str(), code.size());
    sprintf(Argument3, "%08d", len);
    memcpy(&data[24], Argument3, 8);

    int sRet = this->printerToolReadWriteAgent->Send(data, 32);
    if (sRet == 32) {
        if (len == 0 || sData == nullptr) {
            return sRet;
        }
        return this->printerToolReadWriteAgent->Send(sData, len);
    }
    return sRet;
}

int escp_printer::CNTRL_Cutter(int value) {
    char data[100]{};
    sprintf(data, "%08d", value);
    return CNTRL(CNTRLType::CUTTER, (unsigned char *) data, 8);
}

int escp_printer::CNTRL_Start() {
    return CNTRL(CNTRLType::START, nullptr, 0);
}

int escp_printer::CNTRL_FullCutterSet(int CarCutSize) {
    char data[100]{};
    sprintf(data, "%03d%03d%03d%03d000\r", CarCutSize, 0, 0, 0);
    return CNTRL(CNTRLType::FULL_CUTTER_SET, (unsigned char *) data, 16);
}

int escp_printer::CNTRL_BuffCntrl(int value) {
    char data[100]{};
    sprintf(data, "%08d", value);
    return CNTRL(CNTRLType::BUFFCNTRL, (unsigned char *) data, 8);
}

/**
 * 设置 打印大小
 * @param width
 * @param height
 * @return
 */
int escp_printer::setPageSize(int width, int height) {
    page.width = width;
    page.height = height;
    return Error_OK;
}

int escp_printer::print_image(cv::Mat *img) {
    if (!isAgentReady())return Error_AgentNotReady;
    if (img->empty())return Error_ParamFail;
#ifdef ENABLE_OPENCV
    Mat image;
    resize(*img, image, Size(page.width, page.height));
    BMP bmp;
    vector<unsigned char> buf;
    imencode(".bmp", image, buf);
    bmp.ReadBmp(buf.data());

    return print_bmp(&bmp);
#else
    logger::instance()->w(TAG, __LINE__, "未开启 OpenCV");
    return Error_FAIL;
#endif
}

int escp_printer::print_mat(cv::Mat *img) {
    if (!isAgentReady())return Error_AgentNotReady;
    if (img->empty())return Error_ParamFail;
#ifdef ENABLE_OPENCV
    Mat image;
    resize(*img, image, Size(page.width, page.height));
#define IsShowImage FALSE
#if IsShowImage
    namedWindow("enhanced", 0);
    resizeWindow("enhanced", 640, 480);
    imshow("enhanced", Car);
    waitKey(0);
    return false;
#endif

#pragma region 图片处理
    int W = 0;
    int H = 0;

    W = image.cols;//2430
    H = image.rows;//1920

    PALLETTE Quad[256];
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    unsigned char border_adjust_data[10] = {0};
    //设定BITMAPFILEHEADER
    memset(&bfh, 0, sizeof(bfh));
    bfh.identity = 0x4d42;
    bfh.file_size = SIZEOF_BITMAPFILEHEADER + sizeof(
            BITMAPINFOHEADER) + sizeof(border_adjust_data) + sizeof(PALLETTE) * 256 +
                    ((W * 8 + 31) / 32 * 4) * H;
    bfh.data_offset = SIZEOF_BITMAPFILEHEADER + sizeof(
            BITMAPINFOHEADER) + sizeof(border_adjust_data) + sizeof(PALLETTE) * 256;

    //设定BITMAPINFOHEADER
    memset(&bih, 0, sizeof(bih));
    bih.header_size = sizeof(bih);
    bih.width = W;
    bih.height = H;
    bih.planes = 1;   //必须为1
    bih.bit_per_pixel = 8;
    bih.compression = 0; //BI_RGB


    //设定调色盘数据Quad
    for (int n = 0; n < 256; n++) {
        Quad[n].blue = n;
        Quad[n].green = n;
        Quad[n].red = n;
        Quad[n].reserved = 0;
    }

    Mat newImage(image), mC, mM, mY;
    vector<Mat> vecRGB;

    cv::cvtColor(image, newImage, cv::COLOR_BGR2RGB);
    int msize0 = newImage.rows * newImage.step;
    split(newImage, vecRGB);

    mC = vecRGB[0]; //R
    mM = vecRGB[1]; //G
    mY = vecRGB[2]; //B

    flip(mC, vecRGB[0], 1);//1代表水平方向旋转180度
    flip(mM, vecRGB[1], 1);//1代表水平方向旋转180度
    flip(mY, vecRGB[2], 1);//1代表水平方向旋转180度



    int msize = mY.rows * mY.step;
#pragma endregion


    int sRet = 0, rRet = 0;
    string data = Get(escp_printer::ModelType::INFO_FREE_PBUFFER);
    if (data.size() < 5 || (data[4] == '0')) {
        logger::instance()->e(TAG, __LINE__, "打印机没有打印空间");
        return Error_FAIL;
    }


#define MY_PRINT TRUE
#if MY_PRINT

    if (CarCutSize > 0) {
        int type = 0;
        switch (type)
        {
        case 1:
            sprintf(data, "%08d", 1);
            break;
        case 2:
            sprintf(data, "%08d", 120);
            break;
        case 0:
        default:
            sprintf(data, "%08d", 0);
            break;
        }
        PutCntrl("CUTTER", (unsigned char*)data, 8);


        sprintf(data, "%03d%03d%03d%03d000\r"
            , CarCutSize
            , 0
            , 0
            , 0
        );
        PutCntrl("FULL_CUTTER_SET", (unsigned char*)data, 16);
    }
#else
    char __data[32] = {0};
    PutImage(MULTICUT, 8);
    {

        sprintf(__data, "%08d", 1
        );
        memset(&__data[8], 0x00, 32 - 8);
        printerToolReadWriteAgent->Send((unsigned char *) __data, 32);
    }

    CNTRL_BuffCntrl(0);
#endif // MY_PRINT

    // Get(data, "INFO", "RQTY");

    //char data[100];
    //sprintf(data, "%08d", 8);
    //PutCntrl("OVERCOAT", (unsigned char*)data, 8);
    //PutCntrl("BUFFCNTRL", (unsigned char*)data, 8);
    //PutCntrl("QTY", (unsigned char*)data, 8);
    CNTRL_Cutter(0);

    {
        PutImage(YPLANE, msize);

        int ret = 0;
        int _Dsize = SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(border_adjust_data) +
                     sizeof(PALLETTE) * 256;
        unsigned char *_D = new unsigned char[_Dsize];
        memcpy(&_D[0], (unsigned char *) &bfh, SIZEOF_BITMAPFILEHEADER);
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER], (unsigned char *) &bih, sizeof(BITMAPINFOHEADER));
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER)], (unsigned char *) &border_adjust_data,
               sizeof(border_adjust_data));
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(border_adjust_data)],
               (unsigned char *) &Quad, sizeof(PALLETTE) * 256);
        ret = printerToolReadWriteAgent->Send(_D, _Dsize);
        delete[] _D;
        if (ret <= 0) {
            return false;
        }
        ret = printerToolReadWriteAgent->Send((unsigned char *) mY.data, msize);
        if (ret <= 0) {
            return false;
        }
    }
    //Get(data, "INFO", "RQTY",true);
    {
        PutImage(MPLANE, msize);

        int ret = 0;
        int _Dsize = SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(border_adjust_data) +
                     sizeof(PALLETTE) * 256;
        unsigned char *_D = new unsigned char[_Dsize];
        memcpy(&_D[0], (unsigned char *) &bfh, SIZEOF_BITMAPFILEHEADER);
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER], (unsigned char *) &bih, sizeof(BITMAPINFOHEADER));
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER)], (unsigned char *) &border_adjust_data,
               sizeof(border_adjust_data));
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(border_adjust_data)],
               (unsigned char *) &Quad, sizeof(PALLETTE) * 256);
        ret = printerToolReadWriteAgent->Send(_D, _Dsize);
        delete[] _D;
        if (ret <= 0) {
            return false;
        }
        ret = printerToolReadWriteAgent->Send((unsigned char *) mM.data, msize);
        if (ret <= 0) {
            return false;
        }
    }
    {
        PutImage(CPLANE, msize);

        int ret = 0;
        int _Dsize = SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(border_adjust_data) +
                     sizeof(PALLETTE) * 256;
        unsigned char *_D = new unsigned char[_Dsize];
        memcpy(&_D[0], (unsigned char *) &bfh, SIZEOF_BITMAPFILEHEADER);
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER], (unsigned char *) &bih, sizeof(BITMAPINFOHEADER));
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER)], (unsigned char *) &border_adjust_data,
               sizeof(border_adjust_data));
        memcpy(&_D[SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(border_adjust_data)],
               (unsigned char *) &Quad, sizeof(PALLETTE) * 256);
        ret = printerToolReadWriteAgent->Send(_D, _Dsize);
        delete[] _D;
        if (ret <= 0) {
            return false;
        }
        ret = printerToolReadWriteAgent->Send((unsigned char *) mC.data, msize);
        if (ret <= 0) {
            return false;
        }
    }


    CNTRL_Start();
    return 0;
#else
    logger::instance()->w(TAG, __LINE__, "未开启 OpenCV");
    return Error_FAIL;
#endif
}

int escp_printer::print_bmp(BMP *bmp) {
    if (!isAgentReady())return Error_AgentNotReady;
    if (bmp == nullptr)return Error_ParamFail;
    string data = Get(escp_printer::ModelType::INFO_FREE_PBUFFER);
    if (data.size() < 5 || (data[4] == '0')) {
        logger::instance()->e(TAG, __LINE__, "打印机没有打印空间");
        return Error_FAIL;
    }
    BMP bmps[3];
    if (bmp->split(bmps) != 0) {
        logger::instance()->e(TAG, __LINE__, "分割图片失败");
        return Error_ParamFail;
    }

    PutImage(MULTICUT, 8);
    {
        char __data[32] = {0};
        sprintf(__data, "%08d", 1);
        memset(&__data[8], 0x00, 32 - 8);
        printerToolReadWriteAgent->Send((unsigned char *) __data, 32);
        CNTRL_BuffCntrl(8);
    }


    unsigned char border_adjust_data[10] = {0};
    ImageType types[3] = {
            ImageType::YPLANE,
            ImageType::MPLANE,
            ImageType::CPLANE,
    };

    for (int i = 0; i < 3; i++) {
        int msize = bmps[i].GetDataSize();
        PutImage(types[i], msize);

        int ret = 0;
        int _Dsize = SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(border_adjust_data) +
                     sizeof(PALLETTE) * 256;
        unsigned char *_D = new unsigned char[_Dsize];
        bmps[i].GetBITMAPFILEHEADER()->file_size += sizeof(border_adjust_data);
        if (true) {
            int offset = bmps[i].WriteBmpInfo(_D);
            memcpy(&_D[offset], (unsigned char *) &border_adjust_data, sizeof(border_adjust_data));
            offset += sizeof(border_adjust_data);
            if (bmps[i].GetBitPerPixel() == 8) {
                memcpy(&_D[offset], (unsigned char *) bmps[i].GetQuad(), sizeof(PALLETTE) * 256);
                offset += sizeof(PALLETTE) * 256;
            }
        } else {
            memcpy(&_D[0], (unsigned char *) bmps[i].GetBITMAPFILEHEADER(), SIZEOF_BITMAPFILEHEADER);
            memcpy(&_D[SIZEOF_BITMAPFILEHEADER], (unsigned char *) bmps[i].GetBITMAPINFOHEADER(),
                   sizeof(BITMAPINFOHEADER));
            memcpy(&_D[SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER)], (unsigned char *) &border_adjust_data,
                   sizeof(border_adjust_data));
            memcpy(&_D[SIZEOF_BITMAPFILEHEADER + sizeof(BITMAPINFOHEADER) + sizeof(border_adjust_data)],
                   (unsigned char *) bmps[i].GetQuad(), sizeof(PALLETTE) * 256);
        }

        bmps[i].GetBITMAPFILEHEADER()->file_size -= sizeof(border_adjust_data);
        logger::instance()->d(TAG, __LINE__, "%s", __FUNCTION__);
        ret = printerToolReadWriteAgent->Send(_D, _Dsize);
        delete[] _D;
        if (ret <= 0) {
            return Error_FAIL;
        }
        logger::instance()->d(TAG, __LINE__, "%s", __FUNCTION__);
        ret = printerToolReadWriteAgent->Send((unsigned char *) bmps[i].GetData(), msize);
        if (ret <= 0) {
            return Error_FAIL;
        }
    }
    logger::instance()->d(TAG, __LINE__, "%s", __FUNCTION__);
    CNTRL_Start();
    return 0;
}

int escp_printer::print_bmpfile(std::string bmp_path) {
    BMP bmp;
    if (!bmp.ReadBmp(bmp_path.c_str())) {
        return -1;
    }
    return print_bmp(&bmp);
}

