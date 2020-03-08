//
// Created by caesar kekxv on 2020/3/7.
//

#include "usb_tool.h"

#ifdef WIN32
#include <libusb/libusb.h>
#else

#include <libusb-1.0/libusb.h>
#include <logger.h>

#endif

using namespace std;
using namespace clangTools;

#ifdef __FILENAME__
const char *usb_tool::TAG = __FILENAME__;
#else
const char *usb_tool::TAG = "usb_tool";
#endif

void usb_tool::list_print() {
    libusb_context *ctx = nullptr;
    int ret = libusb_init(&ctx);
    if (ret < 0) {
        logger::instance()->e(TAG, __LINE__, "Failed to init libusb");
        return;
    }
    libusb_device **devs = nullptr;
    ssize_t cnt;
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0)
        return;//(int) cnt;
    libusb_device *dev;
    int i = 0;
    while ((dev = devs[i++]) != nullptr) {
        struct libusb_device_descriptor desc{};
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            logger::instance()->e(TAG, __LINE__, "failed to get device descriptor");
            return;
        }
        logger::instance()->i(TAG, __LINE__, "%04x:%04x (bus %d, device %d)",
                              desc.idVendor, desc.idProduct,
                              libusb_get_bus_number(dev), libusb_get_device_address(dev));
    }
    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
}

usb_tool::usb_tool(unsigned short vid, unsigned short pid) {
    logger::instance()->i(TAG, __LINE__, "usb_tool");
    this->vid = vid;
    this->pid = pid;
    int ret = libusb_init(&ctx);
    if (ret < 0) {
        logger::instance()->e(TAG, __LINE__, "Failed to init libusb");
        ctx = nullptr;
    }
}

bool usb_tool::Open() {
    if (ctx == nullptr) {
        logger::instance()->e(TAG, __LINE__, "Failed to init libusb ctx = null");
        return false;
    }
    Close();
    int i = 0;
    ssize_t cnt;
    libusb_device *dev;
    libusb_device **devs;

    cnt = libusb_get_device_list(nullptr, &devs);
    if (cnt < 0) {
        logger::instance()->e(TAG, __LINE__, "Failed libusb_get_device_list");
        return false;
    }

    while ((dev = devs[i++]) != nullptr) {
        struct libusb_device_descriptor desc{};
        int ret = libusb_get_device_descriptor(dev, &desc);
        if (ret < 0) {
            logger::instance()->e(TAG, __LINE__, "Failed libusb_get_device_descriptor");
            continue;
        }
        if (desc.idVendor == vid && desc.idProduct == pid) {
            ret = libusb_open(dev, &deviceHandle);
            if (ret < 0) {
                logger::instance()->e(TAG, __LINE__, "Failed libusb_open: %d", ret);
                break;
            }
//#ifndef WIN32
            libusb_detach_kernel_driver(deviceHandle, 0);
//#endif
            ret = libusb_claim_interface(deviceHandle, 0);
            if (ret < 0) {
                libusb_close(deviceHandle);
                deviceHandle = nullptr;
                break;
            }
            logger::instance()->d(TAG, __LINE__, "bNumConfigurations: %d", desc.bNumConfigurations);
            struct libusb_config_descriptor *conf = nullptr;
            for (unsigned long j = 0; j < desc.bNumConfigurations; ++j) {
                ret = libusb_get_config_descriptor(dev, j, &conf);
                if (ret) {
                    logger::instance()->e(TAG, __LINE__, "Couldn't get configuration "
                                                         "descriptor %lu, some information will "
                                                         "be missing", j);
                } else {
                    // printf("bNumberInterfaces = %5u\n", conf->bNumInterfaces);
                    // printf("bConfigurationValue = %5u\n", conf->bConfigurationValue);
                    if (j == 0 && conf != nullptr) {
                        // usb_in_offset = usb_out_offset =
                        //        conf->interface->altsetting->endpoint->bEndpointAddress - LIBUSB_ENDPOINT_IN;
                        logger::instance()->i(TAG, __LINE__, "bEndpointAddress: 0x%02X",
                                              conf->interface->altsetting->endpoint->bEndpointAddress);
                    }
                    libusb_free_config_descriptor(conf);
                }
            }

            break;
        }
    }
    libusb_free_device_list(devs, 1);
    logger::instance()->i(TAG, __LINE__, "open %s", IsOpen() ? "suss" : "fail");
    return IsOpen();
}

bool usb_tool::IsOpen() { return nullptr != deviceHandle; }

void usb_tool::Close() {
    if (IsOpen()) {
        libusb_close(deviceHandle);
    }
    deviceHandle = nullptr;
}

usb_tool::~usb_tool() {
    Close();
    if (ctx != nullptr)
        libusb_exit(ctx);
    ctx = nullptr;
}


int usb_tool::send(std::vector<unsigned char> sData, int timeout) {
    if (!IsOpen()) {
        logger::instance()->e(TAG, __LINE__, "Error Not Open");
        return Error_NotOpen;
    }
    int ret = 0;
    int actual_length = 0;
    do {
        ret = libusb_bulk_transfer(deviceHandle,
                                   LIBUSB_ENDPOINT_OUT + usb_out_offset,
                                   sData.data(),
                                   sData.size(),
                                   &actual_length,
                                   timeout);
        // logger::instance()->d(TAG, __LINE__, "LIBUSB_ENDPOINT_OUT:ret = %d ; actual_length=%d", ret, actual_length);
        if (ret != -1 && ret != -5) {
            is_out_update = true;
            break;
        }
        usb_out_offset++;
    } while (!is_out_update && usb_out_offset < 20);
    // logger::instance()->i(TAG, __LINE__, "LIBUSB_ENDPOINT_OUT:ret = %d ; actual_length=%d", ret, actual_length);
    return ret;
}


int usb_tool::read(std::vector<unsigned char> &data, int max_size, int timeout) {
    if (!IsOpen()) {
        logger::instance()->e(TAG, __LINE__, "Error_NotOpen");
        return Error_NotOpen;
    }
    auto *rData = new unsigned char[max_size + 1];
    memset(rData, 0x00, max_size + 1);
    int ret = 0;
    int actual_length = 0;
    do {
        ret = libusb_bulk_transfer(deviceHandle,
                                   LIBUSB_ENDPOINT_IN + usb_in_offset,
                                   rData,
                                   max_size,
                                   &actual_length,
                                   timeout);
        // logger::instance()->e(TAG, __LINE__, "LIBUSB_ENDPOINT_IN:ret = %d ; actual_length=%d", ret, actual_length);
        if (ret != -1 && ret != -5) {
            is_in_update = true;
            break;
        }
        usb_in_offset++;
    } while (!is_in_update && usb_in_offset < 20);
    // logger::instance()->i(TAG, __LINE__, "LIBUSB_ENDPOINT_IN:ret = %d ; actual_length=%d", ret, actual_length);
    if (ret < 0) {
        delete[]rData;
        return ret;
    }
    data.insert(data.end(), &rData[0], &rData[actual_length]);
    delete[]rData;
    return ret;
}


