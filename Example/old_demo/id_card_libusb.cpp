//
// Created by caesar kekxv on 2020/3/7.
// id card
//

#include <usb_tool.h>
#include <logger.h>
#include <iconv.h>

using namespace clangTools;
using namespace std;

class IdCard : public usb_tool {
public:
    static const char *TAG;
    static const unsigned short devices[1][2];

    enum ResultCode {
        // 和具体命令有关，可能为空操作成功
        ResultSuccess = 0x90,
        // 证/卡芯片管理号寻找证/卡成功
        ResultFindSuccess = 0x9F,
        // 接收数据校验和错
        ResultFailSum = 0x10,
        // 接收数据长度错
        ResultFailLen = 0x11,
        // 接收数据命令错
        ResultFailCmd = 0x21,
        // 越权操作
        ResultFailPower = 0x23,
        // 无法识别的错误
        ResultFailUnknown = 0x24,
        // 证/卡中此项无内容
        ResultFailCardInfo = 0x91,
        // 寻找证/卡失败
        ResultFailFindCard = 0x80,
        // 选取证/卡失败
        ResultFailSelectCard = 0x81,
        // SAM 未经授权
        ResultFailSAMPower = 0x66,
        //  SAM 自检失败
        ResultFailSAMCheck = 0x60,
        // 取随机数失败
        ResultFailRound = 0x47,
        // 读证/卡操作失败
        ResultFailReadCard = 0x41,
        // 无法识别的卡类型
        ResultFailUnknownCard = 0x40,
        // 信息验证错误
        ResultFailInfoCheck = 0x33,
        // SAM 认证证/卡失败
        ResultFailSAMCheckKey2 = 0x32,
        // 证/卡认证 SAM 失败
        ResultFailSAMCheckKey1 = 0x31,
    };
    enum CMD {
        ResetSam = 0x10FF,
        SamStatus = 0x11FF,
        ReadSamManagement = 0x12FF,
        //寻找身份证信息
        FindCard = 0x2001,
        //选取身份证信息
        SelectCard = 0x2002,
        //读取身份证信息(文字+照片信息)
        ReadIDCard = 0x3001,
        //读取身份证信息(文字+照片+指纹特征点信息)
        ReadIDCardFingerInfo = 0x3010,
        //读取身份证追加信息
        ReadIDCardAddInfo = 0x3002,
        ReadIDNo = 0x3005,
    };
    unsigned char Preamble[5] = {0xAA, 0xAA, 0xAA, 0x96, 0x69};

    struct IdInfo {
        //姓名
        char Name[30 + 10];
        //头像
        unsigned char Header[77725];
        //性别
        char Sex[2 + 10];
        //民族
        char Nation[4 + 10];
        //生日
        char Birth[16 + 10];
        //地址
        char Address[70 + 10];
        //身份证号码
        char ID[36 + 10];
        //签发地址
        char Department[30 + 10];
        //生效期
        char Effect[16 + 10];
        //有效期
        char Expire[16 + 10];
        //错误信息
        char ErrMsg[20 + 10];
    };

    /**
     * 寻卡
     * @return
     */
    inline bool find_card() {
        Send(CMD::FindCard);
        std::vector<unsigned char> data;
        int ret = Read(data, 200);
        logger::instance()->d(TAG, __LINE__, "CMD result ：0x%02X", ret);
        if (!data.empty()) {
            std::string text;
            logger::bytes_to_hex_string(data, text);
            logger::instance()->i(TAG, __LINE__, "CMD result ：0x%s", text.c_str());
        }
        return ret == ResultFindSuccess;
    }

    /**
     * 选卡
     * @return
     */
    inline bool select_card() {
        Send(CMD::SelectCard);
        std::vector<unsigned char> data;
        int ret = Read(data, 200);
        logger::instance()->d(TAG, __LINE__, "CMD result ：0x%02X", ret);
        if (!data.empty()) {
            std::string text;
            logger::bytes_to_hex_string(data, text);
            logger::instance()->i(TAG, __LINE__, "CMD result ：0x%s", text.c_str());
        }
        return ret == ResultFindSuccess || ret == ResultSuccess;
    }

    /**
     * 读卡
     * @param idInfo
     * @return
     */
    inline bool read_card(IdInfo &idInfo) {
        Send(CMD::ReadIDCard);
        std::vector<unsigned char> data;
        int ret = Read(data, 2000);
        logger::instance()->d(TAG, __LINE__, "CMD result ：0x%02X", ret);
        if (ret != ResultSuccess)return false;
        if (!data.empty()) {
            std::string text;
            logger::bytes_to_hex_string(data, text);
            logger::instance()->i(TAG, __LINE__, "CMD result ：0x%s", text.c_str());
        }
        if (data[0] != 0x01 || data[1] != 0x00 || data[2] != 0x04 || data[3] != 0x00) {
            return false;
        }
        g2u((char *) &data[4], 30,
            idInfo.Name, 30 + 10);                                             // Name
        g2u((char *) &data[4 + 30], 2,
            idInfo.Sex, 2 + 10);                                               // Sex
        g2u((char *) &data[4 + 32], 4,
            idInfo.Nation, 4 + 10);                                            // Nation
        g2u((char *) &data[4 + 36], 16,
            idInfo.Birth, 6 + 10);                                             // Birth
        g2u((char *) &data[4 + 52], 68,
            idInfo.Address, 70 + 10);                                          // Address
        g2u((char *) &data[4 + 122], 36,
            idInfo.ID, 36 + 10);                                               // ID
        g2u((char *) &data[4 + 158], 30,
            idInfo.Department, 30 + 10);                                       // Department
        g2u((char *) &data[4 + 188], 16,
            idInfo.Effect, 6 + 10);                                            // Effect
        g2u((char *) &data[4 + 204], 16,
            idInfo.Expire, 6 + 10);                                            // Expire

        logger::instance()->i(TAG, __LINE__, "idInfo.Name: %s", idInfo.Name);
        logger::instance()->i(TAG, __LINE__, "idInfo.Sex: %s", idInfo.Sex);
        logger::instance()->i(TAG, __LINE__, "idInfo.Nation: %s", idInfo.Nation);
        logger::instance()->i(TAG, __LINE__, "idInfo.Birth: %s", idInfo.Birth);
        logger::instance()->i(TAG, __LINE__, "idInfo.Address: %s", idInfo.Address);
        logger::instance()->i(TAG, __LINE__, "idInfo.ID: %s", idInfo.ID);
        logger::instance()->i(TAG, __LINE__, "idInfo.Department: %s", idInfo.Department);
        logger::instance()->i(TAG, __LINE__, "idInfo.Effect: %s", idInfo.Effect);
        logger::instance()->i(TAG, __LINE__, "idInfo.Expire: %s", idInfo.Expire);

        return true;
    }

public:

    inline int Send(CMD cmd, int time_out = 0) {
        std::vector<unsigned char> cmds = {(unsigned char) ((cmd >> 8) & 0xFF), (unsigned char) (cmd & 0xFF)};
        return Send(cmds, time_out);
    }

    inline int Send(std::vector<unsigned char> cmd, int time_out = 0) {
        vector<unsigned char> sData;
        int ret = 0;
        int actual_length = 0;
        sData.insert(sData.end(), &Preamble[0], &Preamble[5]);
        sData.push_back(0);
        sData.push_back(cmd.size() + 1);
        sData.insert(sData.end(), cmd.begin(), cmd.end());
        sData.push_back(getChkSum(sData, sData.size() - 5, 5));
        usb_tool::send(sData, time_out);
        return ret;
    }

    inline int Read(std::vector<unsigned char> &data, int time_out = 0) {
        int ret = 0;
        int actual_length = 0;
        std::vector<unsigned char> rData;
        ret = usb_tool::read(rData, 2080);
        actual_length = rData.size();
        if (ret < 0)return ret;
        if (actual_length < 10)return ErrorCode::Error_FailLen;
        for (int i = 0; i < 5; i++) {
            if (rData[i] != Preamble[i]) {
                return ErrorCode::Error_Fail;
            }
        }
        int r5 = rData[5];
        int len = ((r5 << 8) & 0xFF00) + (rData[6] & 0xFF);
        if (len + 7 != actual_length) {
            return ErrorCode::Error_FailLen;
        }
        int result = 0;
        // memcpy(&((unsigned char*)(void*)&result)[1],&rData[7],3);
        result = rData[9];
        data.insert(data.end(), &rData[10], &rData[10 + len - 1]);
        return result;
    }

private:
    libusb_device_handle *deviceHandle = nullptr;
    libusb_context *ctx = nullptr;

    bool is_out_update = false;
    bool is_in_update = false;
    int usb_out_offset = 0;
    int usb_in_offset = 0;

    static unsigned char getChkSum(std::vector<unsigned char> data, int len, int offset);

#ifndef WIN32

    static int code_convert(char *from_charset, char *to_charset,
                            char *inBuff, size_t inlen, char *outbuf, size_t outlen);

#endif

    static int g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen);

public:
    inline explicit IdCard(unsigned short vid = devices[0][0], unsigned short pid = devices[0][1]) : usb_tool(vid,
                                                                                                              pid) {

    }
};

#ifdef __FILENAME__
const char *IdCard::TAG = __FILENAME__;
#else
const char *IdCard::TAG = "IdCard";
#endif
const unsigned short IdCard::devices[1][2] = {
        {0x0400, 0xc35a}
};


unsigned char IdCard::getChkSum(std::vector<unsigned char> data, int len, int offset) {
    if (len == 0 || offset >= data.size()) {
        return 0;
    }
    unsigned char sum = data[0 + offset];
    for (int i = 1; i < len && i + offset < data.size(); i++) {
        sum ^= data[i + offset];
    }
    return sum;
}

#ifndef WIN32

int IdCard::code_convert(char *from_charset, char *to_charset,
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
        logger::instance()->e(TAG, __LINE__, "errno=%d", errno);
        iconv_close(cd);
        return -1;
    }
    iconv_close(cd);
    return 0;
}

#endif

int IdCard::g2u(char *inbuf, size_t inlen, char *outbuf, size_t outlen) {
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

int main(int argc, char *argv[]) {
    IdCard::IdInfo idInfo{};

    logger::instance()->init_default();
    usb_tool::list_print();

    IdCard idCard;
    idCard.Open();

    idCard.find_card();
    idCard.select_card();
    idCard.read_card(idInfo);

    return 0;
}