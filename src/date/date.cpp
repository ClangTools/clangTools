//
// Created by caesar kekxv on 2024/6/14.
//

#include "date.hpp"

#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
using namespace clangTools;

std::shared_ptr<date> date::now() {
  return std::make_shared<date>();
}

std::shared_ptr<date> date::parse(const int year, const int monthIndex, const int day, const int hours, const int minutes, const int seconds, const int milliseconds) {
  struct tm time{};
  time.tm_year = year - 1900;
  time.tm_mon = monthIndex;
  time.tm_mday = day;
  time.tm_hour = hours;
  time.tm_min = minutes;
  time.tm_sec = seconds;
  calc_tm(&time);
  auto date_ = parse(std::mktime(&time));
  date_->m_time_point = date_->m_time_point + std::chrono::milliseconds(milliseconds);
  return date_;
}

date::~date() = default;

/**
 * 重置时间点
 */
void date::reset() {
  m_time_point = system_clk::now();
}

/**
 * 解析时间
 * @param src 时间
 * @param format_string 时间格式
 * @return 是否解析成功
 */
std::shared_ptr<date> date::parse(const std::string &src, const char *format_string) {
  std::stringstream ss{src};
  std::tm dt{};
  ss >> std::get_time(&dt, replace_format(format_string).c_str());
  const time_t c_time_t = std::mktime(&dt);
  return parse(c_time_t);
}

/**
 * 解析时间戳，只支持秒
 * @param time_ 时间戳
 * @return 是否解析成功
 */
std::shared_ptr<date> date::parse(const std::time_t &time_) {
  auto date_ = now();
  date_->m_time_point = system_clk::from_time_t(time_);
  return date_;
}

/**
 * 解析时间戳，只支持毫秒
 * @param time_ 时间戳
 * @return 是否解析成功
 */
std::shared_ptr<date> date::parseMicroseconds(const std::time_t &time_) {
  return parse(time_ / 1000);
}

std::shared_ptr<date> date::clone(const char *format_string) const {
  const auto t = format(format_string);
  auto format_string_ = std::regex_replace(format_string, std::regex{"--"}, "｜｜");
  format_string_ = std::regex_replace(format_string_, std::regex{"[+-]\\d+"}, "");
  format_string_ = std::regex_replace(format_string_, std::regex{"｜｜"}, "-");
  return parse(t, format_string_.c_str());
}

/**
 * 时间
 * @param format_string 解析格式
 * @return 格式化的时间
 */
std::string date::format(const char *format_string) const {
  auto format_string_ = replace_format(format_string);
  format_string_ = std::regex_replace(format_string_, std::regex{"--"}, "｜｜");
  const std::regex reg("([YyMdHlmS])([-+]\\d+)");
  std::smatch m;
  auto pos = format_string_.cbegin();
  const std::time_t c_time_t = system_clk::to_time_t(m_time_point);
  const auto t_ = std::localtime(&c_time_t);
  struct tm t{};
  memcpy(&t, t_, sizeof(struct tm));
  for (const auto end = format_string_.cend(); std::regex_search(pos, end, m, reg); pos = m.suffix().first) {
    const int offset = static_cast<int>(strtol(m.str(2).c_str(), nullptr, 10));
    switch (m.str(1)[0]) {
      case 'Y':
      case 'y':
        t.tm_year += offset;
        break;
      case 'm':
        t.tm_mon += offset;
        break;
      case 'd':
        t.tm_mday += offset;
        break;
      case 'l':
      case 'H':
        t.tm_hour += offset;
        break;
      case 'M':
        t.tm_min += offset;
        break;
      case 'S':
        t.tm_sec += offset;
        break;
      default:
        break;
    }
  }
  calc_tm(&t);
  format_string_ = std::regex_replace(format_string_, std::regex{"[+-]\\d+"}, "");
  format_string_ = std::regex_replace(format_string_, std::regex{"｜｜"}, "-");
  char mbstr[100]{};
  const size_t size = std::strftime(mbstr, sizeof(mbstr), format_string_.c_str(), &t);
  std::string result{};
  if (size) {
    result.insert(result.end(), mbstr, mbstr + size);
  }
  return result;
}

std::string date::to_string(const char *format_string) const {
  return format(format_string);
}

int date::calc_max_day(const int year, const int mon) {
  int max_day = 31;
  switch (mon + 1) {
    case 2:
      if (year % 100 == 0) {
        max_day = year % 400 == 0 ? 29 : 28;
      } else {
        max_day = year % 4 == 0 ? 29 : 28;
      }
      break;
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      max_day = 31;
      break;
    case 4:
    case 6:
    case 9:
    case 11:
    default:
      max_day = 30;
      break;
  }
  return max_day;
}

void date::calc_tm(struct tm *tm_) {
  if (tm_->tm_sec < 0) { tm_->tm_sec = 0; } else if (tm_->tm_sec >= 60) {
    tm_->tm_sec = tm_->tm_sec - 60;
    tm_->tm_min = tm_->tm_min + 1;
  }
  if (tm_->tm_min < 0) { tm_->tm_min = 0; } else if (tm_->tm_min >= 60) {
    tm_->tm_min = tm_->tm_min - 60;
    tm_->tm_hour = tm_->tm_hour + 1;
  }
  if (tm_->tm_hour < 0) { tm_->tm_hour = 0; } else if (tm_->tm_hour >= 24) {
    tm_->tm_hour = tm_->tm_hour - 24;
    tm_->tm_mday = tm_->tm_mday + 1;
  }
  do {
    const auto max_day = calc_max_day(tm_->tm_year + 1900, tm_->tm_mon);
    if (tm_->tm_mday < 1) { tm_->tm_mday = 1; } else if (tm_->tm_mday > max_day) {
      tm_->tm_mday = tm_->tm_mday - max_day;
      tm_->tm_mon = tm_->tm_mon + 1;
    }
    if (tm_->tm_mon < 0) { tm_->tm_mon = 0; } else if (tm_->tm_mon >= 12) {
      tm_->tm_mon = tm_->tm_mon - 12;
      tm_->tm_year = tm_->tm_year + 1;
      const auto max_day_now = calc_max_day(tm_->tm_year, tm_->tm_mon);
      if (max_day_now != max_day)continue;
    }
    break;
  } while (true);
}

std::string date::replace_format(const char *format_string) {
  auto s = std::regex_replace(format_string, std::regex{"([dHSaAwWzZ])+"}, "%$1");
  if (std::regex_search(s, std::regex{"y{3,}", std::regex::icase})) {
    s = std::regex_replace(s, std::regex{"[Yy]+"}, "%Y");
  } else {
    s = std::regex_replace(s, std::regex{"[Yy]+"}, "%y");
  }
  // 过度变量字符
  s = std::regex_replace(s, std::regex{"m+"}, "、、");
  s = std::regex_replace(s, std::regex{"M+"}, "%m");
  s = std::regex_replace(s, std::regex{"、、"}, "%M");
  s = std::regex_replace(s, std::regex{"h+"}, "%l");
  s = std::regex_replace(s, std::regex{"s+"}, "%S");
  return s;
}

date::date(): m_time_point(system_clk::now()) {
}
