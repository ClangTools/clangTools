//
// Created by caesar kekxv on 2020/8/5.
//

#include <logger.h>
#include <escp_printer.h>
#include <usb_tool.h>

#ifdef ENABLE_OPENCV

#include <opencv2/opencv.hpp>

using namespace cv;
#endif

using namespace clangTools;
using namespace std;

class Agent : public PrinterToolReadWriteAgent, public usb_tool {
public:
    inline Agent(unsigned short vid, unsigned short pid) : usb_tool(vid, pid) {
        Open();
    }

    inline int Send(unsigned char *data, int len, int offset) override {
        std::vector<unsigned char> sData(&data[offset], data + offset + len);
        int ret = this->send(sData, len * 10);
        if (ret < 0) {
            logger::instance()->w(__FILENAME__, __LINE__, "send size %d", ret);
        } else {
            // logger::instance()->d(__FILENAME__, __LINE__, "send size %d:%d", ret, len);
        }
        return ret;
    }

    inline int Read(unsigned char *data, int dataSize, int offset) override {
        std::vector<unsigned char> rData;
        int ret = this->read(rData);
        if (ret > 0) {
            memcpy(&data[offset], rData.data(), rData.size());
        } else {
            // logger::instance()->w(__FILENAME__, __LINE__, "read size %d", ret);
        }
        return ret;
    }
};

void print_info(escp_printer &escpPrinter) {
    string data;
    std::map<escp_printer::ModelType, std::string> info{
            {escp_printer::ModelType::STATUS,             "STATUS"},
            {escp_printer::ModelType::INFO_FVER,          "FVER"},
            {escp_printer::ModelType::INFO_SERIAL_NUMBER, "NUMBER"},
            {escp_printer::ModelType::INFO_MQTY_DEFAULT,  "MDEF"},
            {escp_printer::ModelType::INFO_MQTY,          "MQTY"},
            {escp_printer::ModelType::INFO_RESOLUTION_H,  "RH"},
    };
    for (auto &item : info) {
        data = escpPrinter.Get(item.first);
        logger::instance()->i(__FILENAME__, __LINE__, "%-8s \t : %s (%s)", item.second.c_str(), data.c_str(),
                              escpPrinter.PrinterStatus(data).c_str());
    }
}

int main(int argc, char **argv) {
    logger::instance()->init_default();
    // Mat img = imread("/Users/caesar/Desktop/2.bmp");

    BMP bmp;
#ifdef ENABLE_OPENCV
    Mat img(1088, 1920, CV_8UC3, Scalar(0, 128, 255));
    vector<unsigned char> buf;
    imencode(".bmp", img, buf);
    bmp.ReadBmp(buf.data());
#else
    bmp.ReadBmp("test.bmp");
#endif
    // bmp.WriteBmp("/Users/caesar/Desktop/3.bmp");

    usb_tool::list_print();
    Agent agent(0x1343, 0x0005);
    if (!agent.IsOpen()) {
        logger::instance()->w(__FILENAME__, __LINE__, "打印机没有找到");
        return 1;
    }

    escp_printer escpPrinter(&agent);
    // 重置打印机
    escpPrinter.ResetPrinter();
    // 输出打印机信息
    print_info(escpPrinter);

    // escpPrinter.print_image(&img);
    // escpPrinter.print_mat(&img);
    escpPrinter.print_bmp(&bmp);
    // escpPrinter.print_bmpfile("/Users/caesar/Desktop/2.bmp");
    usleep(1 * 1000 * 1000);
    // 输出打印机信息
    string data;
    {
        data = escpPrinter.Get(escp_printer::ModelType::STATUS);
        if (!data.empty())
            logger::instance()->i(__FILENAME__, __LINE__, "STATUS \t : %s (%s)", data.c_str(),
                                  escpPrinter.PrinterStatus(data).c_str());
    }
    return 0;
}
