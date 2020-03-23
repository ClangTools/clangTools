//
// Created by caesar kekxv on 2020/3/20.
//

#ifndef __TOOLS_IDCARDBASE_H
#define __TOOLS_IDCARDBASE_H

#include <vector>
#include <string>

class IdCardBase {
public:

    struct IdInfo {
        //姓名
        char Name[30 + 10];
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
        //头像
        unsigned char Header[77725];
        //指纹
        unsigned char Fingerprint[1024 + 10];
        //错误信息
        char ErrMsg[20 + 10];
    };

    IdCardBase() = default;

    virtual ~IdCardBase() = default;

    /**
     * get device version
     * @param version
     * @return
     */
    virtual bool GetVersion(std::string &version) = 0;

    /**
     * open device
     * @return
     */
    virtual bool Open() = 0;

    /**
     * check device
     * @return
     */
    virtual bool IsOpen() = 0;

    /**
     * close device
     * @return
     */
    virtual bool Close() = 0;

    /**
     *  find Card
     * @param CardID
     * @return
     */
    virtual long int FindCard(std::vector<unsigned char> &CardID) = 0;

    /**
     * read IDCard info
     * @param IdInfo
     * @return
     */
    virtual long int ReadCard(IdInfo *IdInfo) = 0;
};

#endif //__TOOLS_IDCARDBASE_H
