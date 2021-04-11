//
// Created by caesar kekxv on 2021/4/10.
//

#ifndef TERMINALSERVICE_LOGGING_H
#define TERMINALSERVICE_LOGGING_H

#include "string"
#include "LOGGER_ATTR.h"

class DLL_logger_Export logger;

class Logging {
public:
    /**
     * log rank
     */
    typedef enum log_rank {
        log_rank_NONE = 0,
        log_rank_FATAL = 1,
        log_rank_ERROR = 2,
        log_rank_WARNING = 3,
        log_rank_INFO = 4,
        log_rank_DEBUG = 5,
    } log_rank_t;
    log_rank_t min_level = log_rank_DEBUG;
private:
    std::string TAG;

    /**
     * 日志
     */
    static logger *logger;

public:
    /**
     *
     * @param TAG
     */
    explicit Logging(const char *TAG, log_rank_t min_level = log_rank_DEBUG);

    Logging(const char *TAG, const char *logName, log_rank_t min_level, unsigned long logger_file_max_size,
            unsigned long logger_files_max_size);

    /**
     * 输出调试信息
     * @param fmt 格式代码
     * @param ...
     */
    void debug(_Printf_format_string_ const char *format, ...) const LOGGER_ATTR(2, 3);

    void debug(const char *name, unsigned char *bin, unsigned long bin_size);

    /**
     * 输出信息
     * @param fmt 格式代码
     * @param ...
     */
    void info(_Printf_format_string_ const char *format, ...) LOGGER_ATTR(2, 3);

    void info(const char *name, unsigned char *bin, unsigned long bin_size);


    /**
     * 输出警告信息
     * @param fmt 格式代码
     * @param ...
     */
    void warn(_Printf_format_string_ const char *format, ...) LOGGER_ATTR(2, 3);

    void warn(const char *name, unsigned char *bin, unsigned long bin_size);


    /**
     * 输出错误信息
     * @param fmt 格式代码
     * @param ...
     */
    void error(_Printf_format_string_ const char *format, ...) LOGGER_ATTR(2, 3);

    void error(const char *name, unsigned char *bin, unsigned long bin_size);


    /**
     * 强制输出信息
     * @param fmt 格式代码
     * @param ...
     */
    void fatal(_Printf_format_string_ const char *format, ...) LOGGER_ATTR(2, 3);

    void fatal(const char *name, unsigned char *bin, unsigned long bin_size);

public:
    static void bytes_to_hex_string(const unsigned char *bytes, unsigned long bytes_len, std::string &hex_string);

    static void bytes_to_hex_string(const std::vector<unsigned char> &bytes, std::string &hex_string);

    static void hex_string_to_bytes(const std::string &hex_string, std::vector<unsigned char> &bytes);

    static std::string trim(std::string &str, char c = ' ');

private:
    void init(const char *TAG, const char *logName, log_rank_t min_level, unsigned long logger_file_max_size,
              unsigned long logger_files_max_size);
};


#endif //TERMINALSERVICE_LOGGING_H
