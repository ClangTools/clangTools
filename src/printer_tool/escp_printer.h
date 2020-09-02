//
// Created by caesar kekxv on 2020/8/5.
//

#ifndef TOOLS_ESCP_PRINTER_H
#define TOOLS_ESCP_PRINTER_H


#include <string>
#include <logger.h>
#include "PrinterToolReadWriteAgent.h"
#include <BmpTool.h>

namespace cv {
    class Mat;
}

namespace clangTools {
    class _DLL_ClangTools_Export escp_printer {
    public:
        static const char *TAG;
        enum Error {
            Error_OK = 0,
            Error_FAIL = -1,
            Error_ParamFail = -9,
            Error_AgentNotReady = -9,
        };
        enum ModelType {
            /**
             * Get printer status
             * 获取打印机状态
             */
            STATUS,
            /**
             * Get printer version information
             * 获取打印机版本信息
             */
            INFO_FVER,
            /**
             * Get printer media information
             * 获取打印机介质信息
             */
            INFO_MEDIA,
            /**
             * Get printer horizontal resolution
             * 获取打印机水平分辨率
             */
            INFO_RESOLUTION_H,
            /**
             * Get printer vertical resolution
             * 获取打印机垂直分辨率
             */
            INFO_RESOLUTION_V,
            /**
             * Get number of free print buffers
             * 获取可用打印缓冲区的数量
             */
            INFO_FREE_PBUFFER,
            /**
             * Get default print quantity
             * 获取默认打印数量
             */
            INFO_MQTY_DEFAULT,
            /**
             * Get remaining print quantity
             * 获取剩余打印量
             */
            INFO_MQTY,
            /**
             * 会卡死
             * Get Half Size Conversion Media Counter of Remaining Sheets
             * 获取剩余纸张的半尺寸转换介质计数器
             */
            INFO_RQTY,
            /**
             * Get Printer Serial Number
             * 获取打印机序列号
             */
            INFO_SERIAL_NUMBER,
        };
        enum CNTRLType {
            /**
             * 打印开始
             * Print start
             */
            START,
            /**
             * 切刀控制
             * Cutter control
             */
            CUTTER,
            /**
             * Overcoat finish
             */
            OVERCOAT,
            /**
             * 打印重试控制
             * Print re-try control
             */
            BUFFCNTRL,
            /**
             * 双面打印单元：取消背面打印和弹出
             * Duplex unit: Cancel back print & eject
             */
            DUPLEX_CANCEL,
            /**
             * 完整切刀设置
             * Full Cutter Set-up
             */
            FULL_CUTTER_SET
        };
        enum ImageType {
            YPLANE,
            MPLANE,
            CPLANE,

            MULTICUT,
        };

        struct PageSize {
            int width = 0;
            int height = 0;
        };
    private:
        PrinterToolReadWriteAgent *printerToolReadWriteAgent = nullptr;
        struct PageSize page{1088, 1920};

    private:
        void initSendData(unsigned char *data, const char *model);

        bool PutImage(ImageType type, int len);

    public:
        explicit escp_printer(PrinterToolReadWriteAgent *agent);

        void setPrinterToolReadWriteAgent(PrinterToolReadWriteAgent *agent);

        /**
         * 判断发送接收代理是否设置
         * @return
         */
        bool isAgentReady();

        /**
         * 重置打印机
         * @return
         */
        bool ResetPrinter();

        /**
         * 获取设备信息
         * @param Data 结果内容
         * @param model 需要获取的类型
         * @param readFlag 是否读取结果
         * @return
         */
        int Get(std::string &Data, ModelType model, bool readFlag = true);

        /**
         * 获取设备信息
         * @param model 需要获取的类型
         * @return 结果内容
         */
        std::string Get(ModelType model);

        /**
         * 获取打印机状态含义
         * @param code
         * @return
         */
        std::string PrinterStatus(const std::string &code);

        /**
         * 控制命令
         * @param type
         * @param data
         * @param len
         * @return
         */
        int CNTRL(CNTRLType type, unsigned char *data, int len);

        /**
         * 切刀控制
         * @param value
         * @return
         */
        int CNTRL_Cutter(int value = 0);

        /**
         * 完整切刀设置 并不是所有打印机都支持
         * @param CarCutSize
         * @return
         */
        int CNTRL_FullCutterSet(int CarCutSize = 33);

        /**
         * 打印重试控制
         * @param value
         * @return
         */
        int CNTRL_BuffCntrl(int value = 0);

        /**
         * 控制开始打印
         * @return
         */
        int CNTRL_Start();

        /**
         * 设置 打印大小
         * @param width
         * @param height
         * @return
         */
        int setPageSize(int width = 1088,int height = 1920);
        /**
         * 打印 Mat 图片
         * @return
         */
        int print_image(cv::Mat *img);
        /**
         * 打印 Mat 图片
         * @return
         */
        int print_mat(cv::Mat *img);

        /**
         * 打印 BMP 格式文件
         * @param bmp
         * @return
         */
        int print_bmp(BMP *bmp);

        /**
         * 打印 BMP 格式文件
         * @param bmp
         * @return
         */
        int print_bmpfile(std::string bmp_path);
    };

}
#endif //TOOLS_ESCP_PRINTER_H
