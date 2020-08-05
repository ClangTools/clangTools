//
// Created by caesar kekxv on 2020/8/5.
//

#include <logger.h>
#include <escp_printer.h>
#include <usb_tool.h>

using namespace clangTools;
using namespace std;

class Agent : public PrinterToolReadWriteAgent, public usb_tool {
public:
    inline Agent(unsigned short vid, unsigned short pid) : usb_tool(vid, pid) {
        Open();
    }

    inline int Send(unsigned char *data, int len, int offset) override {
        // logger::instance()->puts_info(__FILENAME__, __LINE__, "send", data, len);
        std::vector<unsigned char> sData(&data[offset], data + offset + len);
        int ret = this->send(sData, 100);
        if (ret < 0)logger::instance()->w(__FILENAME__, __LINE__, "send size %d", ret);
        return ret;
    }

    inline int Read(unsigned char *data, int dataSize, int offset) override {
        std::vector<unsigned char> rData;
        int ret = this->read(rData);
        if (ret > 0) {
            memcpy(&data[offset], rData.data(), rData.size());
        } else {
            logger::instance()->w(__FILENAME__, __LINE__, "read size %d", ret);
        }
        return ret;
    }
};

void print_info(escp_printer &escpPrinter) {
    string data;
    {
        data = escpPrinter.Get(escp_printer::ModelType::STATUS);
        logger::instance()->i(__FILENAME__, __LINE__, "STATUS \t : %s (%s)", data.c_str(),
                              escpPrinter.PrinterStatus(data).c_str());
    }
    {
        data = escpPrinter.Get(escp_printer::ModelType::INFO_FVER);
        logger::instance()->i(__FILENAME__, __LINE__, "FVER   \t : %s", data.c_str());
        data = escpPrinter.Get(escp_printer::ModelType::INFO_SERIAL_NUMBER);
        logger::instance()->i(__FILENAME__, __LINE__, "NUMBER \t : %s", data.c_str());
    }
    {
        data = escpPrinter.Get(escp_printer::ModelType::INFO_MQTY_DEFAULT);
        logger::instance()->i(__FILENAME__, __LINE__, "MDEF   \t : %s",
                              (data.size() > 4 ? data.substr(4) : data).c_str());
        data = escpPrinter.Get(escp_printer::ModelType::INFO_MQTY);
        logger::instance()->i(__FILENAME__, __LINE__, "MQTY   \t : %s",
                              (data.size() > 4 ? data.substr(4) : data).c_str());
    }
    {
        data = escpPrinter.Get(escp_printer::ModelType::INFO_RESOLUTION_H);
        logger::instance()->i(__FILENAME__, __LINE__, "RH     \t : %s",
                              (data.size() > 2 ? data.substr(2) : data).c_str());
        data = escpPrinter.Get(escp_printer::ModelType::INFO_RESOLUTION_V);
        logger::instance()->i(__FILENAME__, __LINE__, "RV     \t : %s",
                              (data.size() > 2 ? data.substr(2) : data).c_str());
    }
}

int main(int argc, char **argv) {
    logger::instance()->init_default();
    usb_tool::list_print();

    Agent agent(0x1343, 0x0005);

    escp_printer escpPrinter(&agent);

    // escpPrinter.ResetPrinter();

    print_info(escpPrinter);

    return 0;
}
