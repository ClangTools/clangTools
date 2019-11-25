//
// Created by caesar on 2019/11/9.
//

#ifndef KTOOL_LOGGER_H
#define KTOOL_LOGGER_H


#include <cstring>
#include <string>
#include <mutex>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <sys/stat.h>
#include<iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
#ifdef _LOGGER_USE_THREAD_POOL_
#include "thread_pool.h"

#ifdef WIN32
#ifdef _Tools_HEADER_
#define DLL_thread_pool_Export  __declspec(dllexport)
#else
#define DLL_thread_pool_Export  __declspec(dllimport)
#endif
#else
#define DLL_thread_pool_Export
#endif

class DLL_thread_pool_Export thread_pool;
#endif

#ifdef WIN32

#include <io.h>
#include <direct.h>
#include <windows.h>

#else
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#endif

#ifdef ANDROID_SO
#include <android/log.h>
#endif

#include <cstdarg>
#include <cstdio>

#ifndef LOGGER_ATTR
#ifdef __GNUC__
#define LOGGER_ATTR(x, y) __attribute__((format(printf, x, y)))
#else
#define LOGGER_ATTR(x, y)
#endif
#endif // LOGGER_ATTR

#ifdef WIN32
#ifdef _Tools_HEADER_
#define DLL_logger_Export  __declspec(dllexport)
#else
#define DLL_logger_Export  __declspec(dllimport)
#endif
#else
#define DLL_logger_Export
#endif

class DLL_logger_Export logger {
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
public:
    log_rank_t min_level = log_rank_NONE;
    bool console_show = false;
    /**
     * file size
     */
    size_t logger_file_max_size = 1024 * 1024;
    /**
     * file max count
     */
    size_t logger_files_max_size = 50;
public:
    /**
     * @return
     */
    static logger *instance();
    static void free_instance();

public:
    /**
     *
     * @param path puts logger to path,if path isn't null
     */
    explicit logger(const char *path = nullptr);

    /**
     *
     * @param path puts logger to path,if path isn't null
     */
    explicit logger(std::fstream *path);

#ifdef _LOGGER_USE_THREAD_POOL_
    void wait_finish();
#endif

    /**
     *
     * @param path puts logger to path,if path isn't null
     */
    void open(const char *path = nullptr);

    /**
     *
     * @param path puts logger to path,if path isn't null
     */
    void open(std::fstream *path);

    bool is_open();

    /**
     * puts info logger
     * @param TAG
     * @param fmt
     * @param ...
     */
    void i(const char *TAG, const char *format, ...) LOGGER_ATTR(3, 4);

    /**
     * puts debug logger
     * @param TAG
     * @param format
     * @param ...
     */
    void d(const char *TAG, const char *format, ...) LOGGER_ATTR(3, 4);

    /**
     * puts warning logger
     * @param TAG
     * @param format
     * @param ...
     */
    void w(const char *TAG, const char *format, ...) LOGGER_ATTR(3, 4);

    /**
     * puts error logger
     * @param TAG
     * @param format
     * @param ...
     */
    void e(const char *TAG, const char *format, ...) LOGGER_ATTR(3, 4);

    /**
     * puts fatal logger
     * @param TAG
     * @param format
     * @param ...
     */
    void f(const char *TAG, const char *format, ...) LOGGER_ATTR(3, 4);

    /**
     * puts info logger
     * @param TAG
     * @param line please use __LINE__
     * @param format a format string
     * @param ...
     */
    void i(const char *TAG, size_t line, const char *format, ...) LOGGER_ATTR(4, 5);

    /**
     * puts debug logger
     * @param TAG
     * @param line please use __LINE__
     * @param format
     * @param ...
     */
    void d(const char *TAG, size_t line, const char *format, ...) LOGGER_ATTR(4, 5);

    /**
     * puts warning logger
     * @param TAG
     * @param line please use __LINE__
     * @param format
     * @param ...
     */
    void w(const char *TAG, size_t line, const char *format, ...) LOGGER_ATTR(4, 5);

    /**
     * puts error logger
     * @param TAG
     * @param line please use __LINE__
     * @param format
     * @param ...
     */
    void e(const char *TAG, size_t line, const char *format, ...) LOGGER_ATTR(4, 5);

    /**
     * puts fatal logger
     * @param TAG
     * @param line please use __LINE__
     * @param format
     * @param ...
     */
    void f(const char *TAG, size_t line, const char *format, ...) LOGGER_ATTR(4, 5);

    void puts_info(const char *TAG, const char *tag_by_data, unsigned char *data, size_t data_len,
                   log_rank_t log_rand_type = log_rank_t::log_rank_DEBUG);

    /**
     * puts log_rank_type logger
     * @param TAG
     * @param data
     * @param log_rank_type
     */
    void puts_info(const char *TAG, const char *data, log_rank_t log_rank_type = log_rank_t::log_rank_INFO);

    /**
     * puts log_rank_type logger
     * @param TAG
     * @param data
     * @param log_rank_type
     */
    void puts_info(const char *TAG, const std::string &data, log_rank_t log_rank_type = log_rank_t::log_rank_INFO);

    ~logger();

private:
    /**
     * puts log_rank_type logger
     * @param log_rank_type
     * @param TAG
     * @param format
     * @param args
     */
    void puts_info(log_rank_t log_rank_type, const char *TAG, const char *format, va_list args);

    /**
     * puts to console
     * @param TAG
     * @param data
     * @param log_rank_type
     */
    void WriteToConsole(const char *TAG, const std::string &data, log_rank_t log_rank_type = log_rank_t::log_rank_INFO);

    /**
     * puts to file,if logger_file isn't null and open
     * @param data
     */
    void WriteToFile(const std::string &data);

    void Free();

private:

#ifdef _LOGGER_USE_THREAD_POOL_
    thread_pool executor{1};
#endif
    bool need_free = false;
    std::string filepath = "";
    std::fstream *logger_file = nullptr;
    std::mutex logger_file_mutex;
    size_t string_max_size = std::string().max_size();
private:
    static const char *L_TAG;
    static std::mutex logger_console_mutex;

public:
#if WIN32
    static char path_split;
#else
    static char path_split;
#endif

    static std::string GetTime(const char *format_string = "%Y-%m-%d %H:%M:%S");

    static long long get_time_tick();

    static std::string vsnprintf(const char *format, va_list args);

    static std::string get_local_path();

    static bool is_dir(const std::string &directory);

    static bool mk_dir(const std::string &directory);

    static void get_files(const std::string &folder_path, std::vector<std::string> &files, int depth = -1) ;

    static void bytes_to_hex_string(const unsigned char *bytes, size_t bytes_len, std::string &hex_string);

    static void bytes_to_hex_string(const std::vector<unsigned char> &bytes, std::string &hex_string);

    static void hex_string_to_bytes(const std::string &hex_string, std::vector<unsigned char> &bytes);

private:
    static int vscprintf(const char *format, va_list pargs);

    static std::string get_path_by_filepath(const std::string &filename);

    static bool _sort_logfile(const std::string &v1, const std::string &v2);

private:
#ifdef WIN32
    //   console color
    enum ConsoleForegroundColor {
        enmCFC_Red = FOREGROUND_INTENSITY | FOREGROUND_RED,
        enmCFC_Green = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
        enmCFC_Blue = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
        enmCFC_Yellow = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
        enmCFC_Purple = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
        enmCFC_Cyan = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
        enmCFC_White = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        enmCFC_Black = 0,
        enmCFC_Default = 0,
    };
    enum ConsoleBackGroundColor {
        enmCBC_Red = BACKGROUND_INTENSITY | BACKGROUND_RED,
        enmCBC_Green = BACKGROUND_INTENSITY | BACKGROUND_GREEN,
        enmCBC_Blue = BACKGROUND_INTENSITY | BACKGROUND_BLUE,
        enmCBC_Yellow = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN,
        enmCBC_Purple = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
        enmCBC_Cyan = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
        enmCBC_White = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
        enmCBC_Black = 0,
        enmCBC_Default = 0,
    };
#else
    enum ConsoleForegroundColor {
        enmCFC_Red = 31,
        enmCFC_Green = 32,
        enmCFC_Blue = 34,
        enmCFC_Yellow = 33,
        enmCFC_Purple = 35,
        enmCFC_Cyan = 36,
        enmCFC_White = 37,
        enmCFC_Black = 30,
        enmCFC_Default = 0,
    };

    enum ConsoleBackGroundColor {
        enmCBC_Red = 41,
        enmCBC_Green = 42,
        enmCBC_Blue = 44,
        enmCBC_Yellow = 43,
        enmCBC_Purple = 45,
        enmCBC_Cyan = 46,
        enmCBC_White = 47,
        enmCBC_Black = 40,
        enmCBC_Default = 0,
    };

#endif
    static void
    SetConsoleColor(ConsoleForegroundColor foreColor = enmCFC_Default,
                    ConsoleBackGroundColor backColor = enmCBC_Default);
};

#endif //KTOOL_LOGGER_H
