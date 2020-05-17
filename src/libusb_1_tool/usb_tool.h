//
// Created by caesar kekxv on 2020/3/7.
//

#ifndef TOOLS_USB_TOOL_H
#define TOOLS_USB_TOOL_H

#include <vector>

typedef struct libusb_device_handle libusb_device_handle;
typedef struct libusb_context libusb_context;

namespace clangTools {
    class usb_tool {
    public:
        /**
         * TAG 标记
         */
        static const char *TAG;

        /**
         * 输出 USB 列表
         */
        static void list_print();

        enum ErrorCode {
            Error_Fail = -99,
            Error_FailLen,
            Error_NotOpen,
        };

    public:
        /**
         * 初始化
         * @param vid 设置 VID
         * @param pid 设置 PID
         */
        usb_tool(unsigned short vid, unsigned short pid);

        ~usb_tool();

        /**
         * 开启 USB
         * @return
         */
        bool Open();

        /**
         * 是否开启 USB
         * @return
         */
        bool IsOpen();

        /**
         * 关闭 USB
         * @return
         */
        void Close();

        /**
         * send data
         * @param sData
         * @param timeout
         * @return
         */
        int send(std::vector<unsigned char> sData, int timeout);

        /**
         * read usb data
         * @param data
         * @param max_size max size
         * @param timeout time out
         * @return
         */
        int read(std::vector<unsigned char> &data, int max_size = 1024, int timeout = 800);

    private:
        unsigned short vid;
        unsigned short pid;
        libusb_device_handle *deviceHandle = nullptr;
        libusb_context *ctx = nullptr;

        bool is_out_update = false;
        bool is_in_update = false;
        int usb_out_offset = 0;
        int usb_in_offset = 0;
        int INTERFACE = 0x00;
    };
}

#endif //TOOLS_USB_TOOL_H
