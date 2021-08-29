//
// Created by caesar kekxv on 2021/4/10.
//

#include "Logging.h"
#include "logger.h"

/**
 * 日志
 */
class DLL_logger_Export logger *Logging::_logger = nullptr;

Logging::Logging(const char *TAG, Logging::log_rank_t min_level) {
    init(TAG, "logging.log", min_level, 1024 * 1024, 50);
}

Logging::Logging(const char *TAG, const char *logName, log_rank_t min_level, unsigned long logger_file_max_size,
                 unsigned long logger_files_max_size) {
    init(TAG, logName, min_level, logger_file_max_size, logger_files_max_size);
}

void Logging::init(const char *_tag, const char *logName, log_rank_t _min_level, unsigned long logger_file_max_size,
                   unsigned long logger_files_max_size) {
    this->min_level = _min_level;
    if (_tag != nullptr) {
        this->TAG = _tag;
    }
    if (_logger == nullptr) {
        _logger = new class logger();
    }
    if (!_logger->is_open()) {
        _logger->init_default();
        _logger->min_level = logger::log_rank_DEBUG;
        std::string logfile =
                logger::get_local_path() + logger::path_split + "logs" + logger::path_split +
                (logName == nullptr ? "Logging.log" : logName);
        _logger->logger_file_max_size = logger_file_max_size;
        _logger->logger_files_max_size = logger_files_max_size;
        _logger->open(logfile.c_str());
    }

}

void Logging::debug(const char *format, ...) const {
    if (min_level < log_rank_DEBUG)return;
    va_list args;
    va_start(args, format);
    _logger->puts_info(logger::log_rank_DEBUG, TAG.c_str(), format, args);
    va_end(args);
}

void Logging::debug(const char *name, unsigned char *bin, unsigned long bin_size) {
    if (min_level < log_rank_DEBUG)return;
    _logger->puts_info(TAG.c_str(), name, bin, bin_size, logger::log_rank_DEBUG);
}

void Logging::info(const char *format, ...) {
    if (min_level < log_rank_INFO)return;
    va_list args;
    va_start(args, format);
    _logger->puts_info(logger::log_rank_INFO, TAG.c_str(), format, args);
    va_end(args);
}

void Logging::info(const char *name, unsigned char *bin, unsigned long bin_size) {
    if (min_level < log_rank_INFO)return;
    _logger->puts_info(TAG.c_str(), name, bin, bin_size, logger::log_rank_INFO);
}

void Logging::warn(const char *format, ...) {
    if (min_level < log_rank_WARNING)return;
    va_list args;
    va_start(args, format);
    _logger->puts_info(logger::log_rank_WARNING, TAG.c_str(), format, args);
    va_end(args);
}

void Logging::warn(const char *name, unsigned char *bin, unsigned long bin_size) {
    if (min_level < log_rank_WARNING)return;
    _logger->puts_info(TAG.c_str(), name, bin, bin_size, logger::log_rank_WARNING);
}

void Logging::error(const char *format, ...) {
    if (min_level < log_rank_ERROR)return;
    va_list args;
    va_start(args, format);
    _logger->puts_info(logger::log_rank_ERROR, TAG.c_str(), format, args);
    va_end(args);
}

void Logging::error(const char *name, unsigned char *bin, unsigned long bin_size) {
    if (min_level < log_rank_ERROR)return;
    _logger->puts_info(TAG.c_str(), name, bin, bin_size, logger::log_rank_ERROR);
}

void Logging::fatal(const char *format, ...) {
    if (min_level < log_rank_FATAL)return;
    va_list args;
    va_start(args, format);
    _logger->puts_info(logger::log_rank_FATAL, TAG.c_str(), format, args);
    va_end(args);
}

void Logging::fatal(const char *name, unsigned char *bin, unsigned long bin_size) {
    if (min_level < log_rank_FATAL)return;
    _logger->puts_info(TAG.c_str(), name, bin, bin_size, logger::log_rank_FATAL);
}


void Logging::bytes_to_hex_string(const unsigned char *bytes, unsigned long bytes_len, std::string &hex_string) {
    logger::bytes_to_hex_string(bytes, bytes_len, hex_string);
}

void Logging::bytes_to_hex_string(const std::vector<unsigned char> &bytes, std::string &hex_string) {
    logger::bytes_to_hex_string(bytes, hex_string);
}

void Logging::hex_string_to_bytes(const std::string &hex_string, std::vector<unsigned char> &bytes) {
    logger::hex_string_to_bytes(hex_string, bytes);
}

std::string Logging::trim(std::string &str, char c) {
    return logger::trim(str, c);
}

