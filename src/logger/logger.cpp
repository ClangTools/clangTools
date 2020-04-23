//
// Created by caesar on 2019/11/9.
//

#include "logger.h"
#include <vector>

#ifndef WIN32

#include <sys/time.h>

#endif
using namespace std;

#ifdef __FILENAME__
const char *logger::L_TAG = __FILENAME__;
#else
const char *logger::L_TAG = "logger";
#endif

#if WIN32
char DLL_logger_Export logger::path_split = '\\';
#else
char logger::path_split = '/';
#endif

logger *_logger = nullptr;

logger *logger::instance() {
    if (_logger == nullptr) {
        _logger = new logger{};
    }
    return _logger;
}

void logger::free_instance() {
    delete _logger;
    _logger = nullptr;
}

std::mutex logger::logger_console_mutex;

logger::logger(const char *path) {
    open(path);
}

logger::logger(FILE *path) {
    open(path);
}

void logger::open(const char *path) {
    Free();
    if (path != nullptr) {
        mk_dir(get_path_by_filepath(path));
        logger_file = fopen( path, "ab+" );;
        need_free = true;
        // logger_file->open(path, ios::app);
        this->filepath = path;
    }
}

void logger::open(FILE *path) {
    Free();
    need_free = false;
    logger_file = path;
}

logger::~logger() {
#ifdef _LOGGER_USE_THREAD_POOL_
    wait_finish();
#endif
    Free();
    std::lock_guard<std::mutex> guard1(logger_console_mutex);
}

#ifdef _LOGGER_USE_THREAD_POOL_

void logger::wait_finish() {
    executor.wait_finish();
}

#endif

void logger::Free() {
    std::lock_guard<std::mutex> guard1(logger_file_mutex);
    if (need_free) {
        need_free = true;
        fclose(logger_file);
        logger_file = nullptr;
    }
}

void logger::WriteToFile(const std::string &data) {
    std::lock_guard<std::mutex> guard(logger_file_mutex);
    if (!is_open()) {
        //printf("%s:%d\n", __FILENAME__, __LINE__);
        return;
    }
    fwrite(data.c_str(), data.size(),1,logger_file);
    if (data.find('\n') == string::npos) return;
    fflush(logger_file);
    if (logger_file_max_size <= 0) return;
    if (filepath.empty()) return;
    if (!need_free) return;
    fseek(logger_file,0,SEEK_SET);
    fseek(logger_file,0,SEEK_END);
    ssize_t dst_file_size = ftell(logger_file);
    // printf("%zd : %zu",dst_file_size,logger_file_max_size);
    if (dst_file_size < logger_file_max_size)return;
    string path = get_path_by_filepath(filepath);
    if (path.empty()) {
        logger_file_max_size = 0;
        return;
    }
    if (!is_dir(path)) {
        logger_file_max_size = 0;
        return;
    }
    fclose(logger_file);
    logger_file = nullptr;

    string filename = filepath.substr(path.size() + 1);
    auto _index = filename.rfind('.');
    time_t timep;
#ifdef _WIN32
#define EPOCHFILETIME   (116444736000000000UL)
    FILETIME ft;
    LARGE_INTEGER li;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    timep = (li.QuadPart - EPOCHFILETIME) / 10;
#else
    timeval tv{};
    gettimeofday(&tv, 0);
    timep = (int64_t) tv.tv_sec * 1000000 + (int64_t) tv.tv_usec;
#endif // _WIN32
    string new_filename =
            filename + GetTime(("_%Y-%m-%d_%H-%M-%S" + to_string(10000 + timep % 10000) + ".log").c_str());
    string l_filename = filename.substr(0, _index);
    if (_index != string::npos) {
        new_filename = l_filename + GetTime("_%Y-%m-%d_%H-%M-%S_") + to_string(10000 + timep % 10000) +
                       filename.substr(_index);
    }
    int ret = rename((path + path_split + filename).c_str(), (path + path_split + new_filename).c_str());
    if (ret != 0) {
        logger_file_max_size = 0;
    }
    need_free = true;
    logger_file = fopen(filepath.c_str(), "ab+");
    if (logger_files_max_size <= 0)return;
    if (logger_file_max_size > 0) {
        std::vector<std::string> files, log_files;
        get_files(path, files, 0);
        for (const auto &item:files) {
            if (item.substr(path.size() + 1).find(l_filename + "_") != string::npos) {
                log_files.push_back(item);
            }
        }
        if (logger_files_max_size > log_files.size())return;
        std::sort(log_files.begin(), log_files.end(), _sort_logfile);
        while (logger_files_max_size <= log_files.size() && !log_files.empty()) {
            if (remove(log_files[log_files.size() - 1].c_str()) != 0) {
                logger_files_max_size = 0;
                return;
            }
            log_files.pop_back();
        }
    }
}

void logger::WriteToConsole(const char *TAG, const std::string &data, log_rank_t log_rank_type) {
    if (min_level < log_rank_type)return;

#ifdef _LOGGER_USE_THREAD_POOL_
    string _tag(TAG);
    auto fh = executor.commit(
            [this](const string &_tag, const std::string &data, log_rank_t log_rank_type) -> void {
#endif
    std::lock_guard<std::mutex> guard(logger_console_mutex);
#ifdef _LOGGER_USE_THREAD_POOL_
    const char *TAG = _tag.c_str();
#endif
#ifdef WIN32
    HANDLE handle = nullptr;
    WORD wOldColorAttrs = 0;
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo{};
    if (console_show) {
        handle = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(handle, &csbiInfo);
        wOldColorAttrs = csbiInfo.wAttributes;
    }
#endif
#ifdef ANDROID_SO
    __android_log_print(log_rank_type > log_rank_t::log_rank_ERROR ? ANDROID_LOG_INFO : ANDROID_LOG_ERROR, TAG, "%s", data.c_str());
#endif

    string _time = GetTime("%Y%m%d %H:%M:%S");
    if (console_show) {
        SetConsoleColor(ConsoleForegroundColor::enmCFC_Blue, ConsoleBackGroundColor::enmCBC_Default);
        printf("%s", _time.c_str());
        SetConsoleColor(ConsoleForegroundColor::enmCFC_Default);
    }

    WriteToFile(_time);

    string _type = "I";
    switch (log_rank_type) {
        case log_rank_t::log_rank_DEBUG:
            _type = "D";
            break;
        case log_rank_t::log_rank_WARNING:
            _type = "W";
            break;
        case log_rank_t::log_rank_ERROR:
            _type = "E";
            break;
        case log_rank_t::log_rank_FATAL:
            _type = "F";
            break;
        case log_rank_t::log_rank_INFO:
        default:
            _type = "I";
            break;
    }
    if (console_show) {
        SetConsoleColor(ConsoleForegroundColor::enmCFC_Red, ConsoleBackGroundColor::enmCBC_Yellow);
        printf("[%s]", _type.c_str());
        SetConsoleColor(ConsoleForegroundColor::enmCFC_Default);
    }
    WriteToFile("[" + _type + "]");

    if (TAG != nullptr) {
        if (console_show) {
            SetConsoleColor(ConsoleForegroundColor::enmCFC_Blue, ConsoleBackGroundColor::enmCBC_Cyan);
            printf("[ %s ]", TAG);
            SetConsoleColor(ConsoleForegroundColor::enmCFC_Default);
        }
        WriteToFile("[" + string(TAG) + "]");
    }

    if (console_show) {
        switch (log_rank_type) {
            case log_rank_t::log_rank_DEBUG:
                SetConsoleColor(ConsoleForegroundColor::enmCFC_Cyan,
                                ConsoleBackGroundColor::enmCBC_Default);
                break;
            case log_rank_t::log_rank_WARNING:
                SetConsoleColor(ConsoleForegroundColor::enmCFC_Yellow,
                                ConsoleBackGroundColor::enmCBC_Default);
                break;
            case log_rank_t::log_rank_ERROR:
                SetConsoleColor(ConsoleForegroundColor::enmCFC_Red, ConsoleBackGroundColor::enmCBC_Default);
                break;
            case log_rank_t::log_rank_FATAL:
                SetConsoleColor(ConsoleForegroundColor::enmCFC_Purple,
                                ConsoleBackGroundColor::enmCBC_Default);
                break;
            case log_rank_t::log_rank_INFO:
            default:
                SetConsoleColor(ConsoleForegroundColor::enmCFC_Green,
                                ConsoleBackGroundColor::enmCBC_Default);
                break;
        }

        printf("%s", data.c_str());
        SetConsoleColor(ConsoleForegroundColor::enmCFC_Default);
    }
    WriteToFile(data);
#ifdef WIN32
    SetConsoleTextAttribute(handle, wOldColorAttrs);
#endif
    if (console_show) {
        printf("\n");
    }
    WriteToFile("\n");

#ifdef _LOGGER_USE_THREAD_POOL_
    }, _tag, data, log_rank_type);
if (wait_show)fh.get();
#endif
}

void logger::puts_info(const char *TAG, const char *data, log_rank_t log_rank_type) {
    if (min_level < log_rank_type)return;
    if (data == nullptr)return;
    if (strlen(data) >= string_max_size)return;
    puts_info(TAG, data, log_rank_type);
}

void logger::puts_info(const char *TAG, const std::string &data, log_rank_t log_rank_type) {
    if (min_level < log_rank_type)return;
    if (data.empty())return;
    WriteToConsole(TAG, data, log_rank_type);
}

void logger::i(const char *TAG, const char *format, ...) {
    if (min_level < log_rank_INFO)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_INFO, TAG, format, args);
    va_end(args);
}

void logger::d(const char *TAG, const char *format, ...) {
    if (min_level < log_rank_DEBUG)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_DEBUG, TAG, format, args);
    va_end(args);
}

void logger::w(const char *TAG, const char *format, ...) {
    if (min_level < log_rank_WARNING)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_WARNING, TAG, format, args);
    va_end(args);
}

void logger::e(const char *TAG, const char *format, ...) {
    if (min_level < log_rank_ERROR)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_ERROR, TAG, format, args);
    va_end(args);
}

void logger::f(const char *TAG, const char *format, ...) {
    if (min_level < log_rank_FATAL)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_FATAL, TAG, format, args);
    va_end(args);
}

void logger::i(const char *TAG, size_t line, const char *format, ...) {
    if (min_level < log_rank_INFO)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_INFO, (string(TAG) + ":" + to_string(line)).c_str(), format, args);
    va_end(args);
}

void logger::d(const char *TAG, size_t line, const char *format, ...) {
    if (min_level < log_rank_DEBUG)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_DEBUG, (string(TAG) + ":" + to_string(line)).c_str(), format, args);
    va_end(args);
}

void logger::w(const char *TAG, size_t line, const char *format, ...) {
    if (min_level < log_rank_WARNING)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_WARNING, (string(TAG) + ":" + to_string(line)).c_str(), format, args);
    va_end(args);
}

void logger::e(const char *TAG, size_t line, const char *format, ...) {
    if (min_level < log_rank_ERROR)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_ERROR, (string(TAG) + ":" + to_string(line)).c_str(), format, args);
    va_end(args);
}

void logger::f(const char *TAG, size_t line, const char *format, ...) {
    if (min_level < log_rank_FATAL)return;
    va_list args;
    va_start(args, format);
    puts_info(log_rank_FATAL, (string(TAG) + ":" + to_string(line)).c_str(), format, args);
    va_end(args);
}

void logger::puts_info(const char *TAG, int line, const char *tag_by_data, unsigned char *data, size_t data_len,
                       log_rank_t log_rand_type) {
    puts_info((string(TAG) + ":" + to_string(line)).c_str(), tag_by_data, data, data_len, log_rand_type);
}

void logger::puts_info(const char *TAG, const char *tag_by_data, unsigned char *data, size_t data_len,
                       logger::log_rank_t log_rand_type) {
    string _tag_by_data = tag_by_data == nullptr ? "" : tag_by_data;
    string bytes_string;
    bytes_to_hex_string(data, data_len, bytes_string);
    puts_info(TAG, _tag_by_data + ":[" + bytes_string + "]", log_rand_type);
}

void logger::puts_info(log_rank_t log_rank_type, const char *TAG, const char *format, va_list args) {
    if (min_level < log_rank_type)return;
    string _Message = vsnprintf(format, args);
    puts_info(TAG, _Message, log_rank_type);
}

bool logger::is_open() {
    return logger_file != nullptr/* && logger_file->is_open()*/;
}

#ifdef WIN32

void
logger::SetConsoleColor(ConsoleForegroundColor foreColor,
                        ConsoleBackGroundColor backColor) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(handle, foreColor | backColor);
}

#else

void logger::SetConsoleColor(ConsoleForegroundColor foreColor,
                             ConsoleBackGroundColor backColor) {
    if (enmCFC_Default == foreColor && enmCBC_Default == backColor) {
        printf("\x1b[0m");
    } else {
        if (enmCBC_Default == backColor) {
            printf("\x1b[%d;%dm", backColor, foreColor);
        } else {
            printf("\x1b[%d;%dm", foreColor, backColor);
        }
    }
}

#endif

std::string logger::GetTime(const char *format_string) {
    time_t timep;
    time(&timep);
    char tmp[128] = {};
    strftime(tmp, sizeof(tmp), format_string, localtime(&timep));
    return tmp;
}

std::string logger::vsnprintf(const char *format, va_list args) {
    std::string data;
    auto max_size = vscprintf(format, args) + 1;
    char *_Message = new char[max_size * sizeof(char)];
    memset(_Message, 0, max_size * sizeof(char));
    ::vsnprintf(_Message, max_size * sizeof(char), format, args);
    data = _Message;
    delete[]_Message;
    return data;
}

std::string logger::get_local_path() {
#ifdef WIN32
    char *path = new char[MAX_PATH * 2];
    HMODULE hm = nullptr;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                          GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          (LPCSTR) &get_local_path, &hm) == 0) {
        delete[]path;
        return "";
    }
    if (GetModuleFileName(hm, path, MAX_PATH * 2) == 0) {
        delete[]path;
        return "";
    }
    // The path variable should now contain the full filepath for this DLL.
    string _path = path;
    delete[]path;
    return get_path_by_filepath(_path);
#else
    Dl_info dlInfo;
    dladdr((const void *) get_local_path, &dlInfo);
    if (dlInfo.dli_fname != nullptr) {
        auto path = get_path_by_filepath(dlInfo.dli_fname);
        if (path[0] != path_split) {
            return "./";
        }
        return path;
    } else
        return "";
#endif
}

bool logger::is_dir(const std::string &directory) {
#ifdef WIN32
    DWORD dwAttr = GetFileAttributes(directory.c_str());
    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
    {//是目录
        return true;
    }
    else
    {//是文件
        return false;
    }
    // return (_access(directory.c_str(), 0) == 0);
#else
    struct stat buf{};
    if (lstat(directory.c_str(), &buf) < 0) {
        return false;
    }
    int ret = S_IFDIR & buf.st_mode;
    return ret != 0;
    // return (access(directory.c_str(), 0) == 0);
#endif
}

bool logger::mk_dir(const std::string &directory) {
#ifdef WIN32
    return (_mkdir(directory.c_str()) == 0);
#else
    return (mkdir(directory.c_str(), 0755) == 0);
#endif
}

void logger::get_files(const std::string &folder_path, std::vector<std::string> &files, int depth) {
#ifdef WIN32
    //intptr_t hFile = 0;//Win10
    long hFile = 0;
    struct _finddata_t fileinfo{};
    std::string p;
    try {
        if ((hFile = _findfirst(p.assign(folder_path).append("\\*").c_str(), &fileinfo)) != -1) {
            do {
                if ((fileinfo.attrib & _A_SUBDIR)) {
                    if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                        if (depth == -1 || depth > 0)
                            get_files(p.assign(folder_path).append("\\").append(fileinfo.name), files, depth - 1);
                } else {
                    files.push_back(p.assign(folder_path).append("\\").append(fileinfo.name));
                }
            } while (_findnext(hFile, &fileinfo) == 0);

            _findclose(hFile);
        }
    }
    catch (std::exception e) {
    }
#else
    DIR *dir;
    struct dirent *ptr;

    if ((dir = opendir(folder_path.c_str())) == nullptr) {
        return;
    }

    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    // current dir OR parent dir
        {
            continue;
        } else if (ptr->d_type == 2)    // file
        {
            files.push_back(folder_path + path_split + ptr->d_name);
        } else if (ptr->d_type == 6)    // file
        {
            files.push_back(folder_path + path_split + ptr->d_name);
        } else if (ptr->d_type == 8)    // file
        {
            files.push_back(folder_path + path_split + ptr->d_name);
        } else if (ptr->d_type == 10)    // link file
        {
            files.push_back(folder_path + path_split + ptr->d_name);
        } else if (ptr->d_type == 4)    // dir
        {
            files.push_back(folder_path + path_split + ptr->d_name);
            if (depth == -1 || depth > 0)
                get_files(folder_path + path_split + ptr->d_name, files);
        }
    }
    closedir(dir);
#endif
}

void logger::bytes_to_hex_string(const unsigned char *bytes, size_t bytes_len, std::string &hex_string) {
    std::vector<unsigned char> _bytes(bytes, bytes + bytes_len);
    bytes_to_hex_string(_bytes, hex_string);
}

void logger::bytes_to_hex_string(const std::vector<unsigned char> &bytes, std::string &hex_string) {
    unsigned char highByte, lowByte;
    for (const auto &item : bytes) {
        highByte = (item >> 4) & 0x0f;
        lowByte = item & 0x0f;
        highByte += 0x30;
        if (highByte > 0x39)
            hex_string.push_back((char) (highByte + 0x07));
        else
            hex_string.push_back(highByte);
        lowByte += 0x30;
        if (lowByte > 0x39)
            hex_string.push_back(lowByte + 0x07);
        else
            hex_string.push_back(lowByte);
    }
}

void logger::hex_string_to_bytes(const std::string &hex_string, std::vector<unsigned char> &bytes) {
    std::string data = hex_string;
    if (hex_string.size() < 2)return;
    if (data.size() % 2 != 0) {
        data.insert(data.size() - 2, "0");
    }
    char highByte, lowByte;
    for (size_t i = 0; i < data.size(); i += 2) {
        highByte = data[i * 2 + 0];
        lowByte = data[i * 2 + 1];
        if (highByte > 0x39) highByte -= 0x07;
        if (lowByte > 0x39) lowByte -= 0x07;
        bytes.push_back(((highByte << 4) & 0xF0) ^ (lowByte & 0x0F));
    }
}

int logger::vscprintf(const char *format, va_list pargs) {
    int ret_val;
    va_list argcopy;
    va_copy(argcopy, pargs);
    ret_val = ::vsnprintf(nullptr, 0, format, argcopy);
    va_end(argcopy);
    return ret_val;
}

std::string logger::get_path_by_filepath(const std::string &filename) {
    if (filename.empty())return filename;
    std::string directory;
    const size_t last_slash_idx = filename.rfind(path_split);
    if (std::string::npos != last_slash_idx) {
        directory = filename.substr(0, last_slash_idx);
        if (!is_dir(directory)) {
            directory = get_path_by_filepath(directory);
        }
        return directory;
    }
    return filename;
}

bool logger::_sort_logfile(const std::string &v1, const std::string &v2) {
    return v1 > v2;
}

long long logger::get_time_tick() {
#ifdef _WIN32
    // 从1601年1月1日0:0:0:000到1970年1月1日0:0:0:000的时间(单位100ns)
#define EPOCHFILETIME (116444736000000000UL)
    FILETIME ft;
    LARGE_INTEGER li;
    long long tt = 0;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    // 从1970年1月1日0:0:0:000到现在的微秒数(UTC时间)
    tt = (li.QuadPart - EPOCHFILETIME) / 10 / 1000;
    return tt;
#else
    timeval tv;
    gettimeofday(&tv, 0);
    return (long long) tv.tv_sec * 1000 + (long long) tv.tv_usec / 1000;
#endif // _WIN32
    return 0;
}

int64_t logger::get_mtime(string filename) {
    struct stat result{};
    if (stat(filename.c_str(), &result) == 0) {
        auto mod_time = result.st_mtime;
        return mod_time;
    }
    return -1;
}

bool logger::StartsWith(const string &text, const string &prefix) {
    return prefix.empty() ||
           (text.size() >= prefix.size() &&
            memcmp(text.data(), prefix.data(), prefix.size()) == 0);
}

bool logger::EndsWith(const string &text, const string &suffix) {
    return suffix.empty() ||
           (text.size() >= suffix.size() &&
            memcmp(text.data() + (text.size() - suffix.size()), suffix.data(),
                   suffix.size()) == 0);
}

void logger::init_default(const std::string &path) {
    this->min_level = logger::log_rank_t::log_rank_DEBUG;
    this->console_show = true;
#ifdef _LOGGER_USE_THREAD_POOL_
    this->wait_show = true;
#endif
    if (!path.empty()) {
        logger::mk_dir(path);
        std::string logfile = path + logger::path_split + "logger.log";
        this->open((logfile).c_str());
        this->logger_files_max_size = 5;
    }
}

bool logger::exists(const std::string &path_string) {
    struct stat buffer{};
    return (stat(path_string.c_str(), &buffer) == 0);
}

string logger::extension(const std::string &path_string) {
    return path_string.substr(path_string.find_last_of('.'));
}

string &logger::replace_all(string &str, const string &old_value, const string &new_value) {
    while (true) {
        string::size_type pos(0);
        if ((pos = str.find(old_value)) != string::npos) {
            str.replace(pos, old_value.length(), new_value);
        } else { break; }
    }
    return str;
}

string &logger::replace_all_distinct(string &str, const string &old_value, const string &new_value) {
    for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
        if ((pos = str.find(old_value, pos)) != string::npos) {
            str.replace(pos, old_value.length(), new_value);
        } else { break; }
    }
    return str;
}


class __logger_free {
public:
    inline __logger_free() = default;;

    inline ~__logger_free() {
        logger::free_instance();
    };
};

__logger_free ___logger_free{};