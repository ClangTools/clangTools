//
// Created by caesar kekxv on 2024/6/14.
//

#ifndef CLANGTOOLS_DATE_HPP
#define CLANGTOOLS_DATE_HPP
#include <istream>
#include <memory>
#include <string>

namespace clangTools {
  class date {
    // 重命名system_clock名称空间
    using system_clk = std::chrono::system_clock;
    // 重命名time_point类型
    using _time_point = std::chrono::time_point<system_clk>;

  public:
    static std::shared_ptr<date> now();

    /**
     * 创建时间对象
     * @param year 年
     * @param monthIndex 月：0-11
     * @param day 天
     * @param hours 小时
     * @param minutes 分钟
     * @param seconds 秒
     * @param milliseconds 毫秒
     * @return 时间
     */
    static std::shared_ptr<date> parse(int year, int monthIndex, int day = 1, int hours = 0, int minutes = 0, int seconds = 0, int milliseconds = 0);

    /**
     * 解析时间
     * @param src 时间
     * @param format_string 时间格式
     * @return 是否解析成功
     */
    static std::shared_ptr<date> parse(const std::string &src, const char *format_string = "yyyy-MM-dd HH:mm:ss");

    /**
     * 解析时间戳，只支持秒
     * @param time_ 时间戳
     * @return 是否解析成功
     */
    static std::shared_ptr<date> parse(const std::time_t &time_);

    /**
     * 解析时间戳，只支持毫秒
     * @param time_ 时间戳
     * @return 是否解析成功
     */
    static std::shared_ptr<date> parseMicroseconds(const std::time_t &time_);

    ~date();

    explicit date();

    /**
     * 重置时间点
     */
    void reset();


    /**
     * 时间
     * @param format_string 解析格式
     * @return 格式化的时间
     */
    std::string format(const char *format_string = "yyyy-MM-dd HH:mm:ss") const;

    /**
     * 转字符串
     * @param format_string 格式
     * @return 字符串
     */
    std::string to_string(const char *format_string = "yyyy-MM-dd HH:mm:ss") const;

    /**
     * 克隆一个对象
     * @param format_string 格式，支持 "yyyy+1-MM-1-dd HH:mm:ss" 如果是 "yyyy-01-01 HH:mm:ss"，需要改为 "yyyy--01--01 HH:mm:ss"，否则解析错误
     * @return 新的对象
     */
    std::shared_ptr<date> clone(const char *format_string = "yyyy-MM-dd HH:mm:ss") const;

  public:
    date &operator=(const date &date_) = default;

    date &operator=(const std::shared_ptr<date> &date_) {
      m_time_point = date_->m_time_point;
      return (*this);
    }

    friend std::ostream &operator<<(std::ostream &output, const date &date_) {
      output << date_.to_string();
      return output;
    }

    friend std::ostream &operator<<(std::ostream &output, const std::shared_ptr<date> &date_) {
      output << date_->to_string();
      return output;
    }

    friend std::istream &operator>>(std::istream &input, date &date_) {
      std::string str;
      input >> str;
      date_ = date::parse(str);
      return input;
    }

    friend bool operator>(const date &d1, const date &d2) { return d1.m_time_point > d2.m_time_point; }
    friend bool operator>(const std::shared_ptr<date> &d1, const std::shared_ptr<date> &d2) { return *d1 > *d2; }
    friend bool operator>(const date &d1, const std::shared_ptr<date> &d2) { return d1 > *d2; }
    friend bool operator>(const std::shared_ptr<date> &d1, const date &d2) { return *d1 > d2; }

    friend bool operator>=(const date &d1, const date &d2) { return d1.m_time_point >= d2.m_time_point; }
    friend bool operator>=(const std::shared_ptr<date> &d1, const std::shared_ptr<date> &d2) { return *d1 >= *d2; }
    friend bool operator>=(const date &d1, const std::shared_ptr<date> &d2) { return d1 >= *d2; }
    friend bool operator>=(const std::shared_ptr<date> &d1, const date &d2) { return *d1 >= d2; }

    friend bool operator<(const date &d1, const date &d2) { return d1.m_time_point < d2.m_time_point; }
    friend bool operator<(const std::shared_ptr<date> &d1, const std::shared_ptr<date> &d2) { return *d1 < *d2; }
    friend bool operator<(const date &d1, const std::shared_ptr<date> &d2) { return d1 < *d2; }
    friend bool operator<(const std::shared_ptr<date> &d1, const date &d2) { return *d1 < d2; }

    friend bool operator<=(const date &d1, const date &d2) { return d1.m_time_point <= d2.m_time_point; }
    friend bool operator<=(const std::shared_ptr<date> &d1, const std::shared_ptr<date> &d2) { return *d1 <= *d2; }
    friend bool operator<=(const date &d1, const std::shared_ptr<date> &d2) { return d1 <= *d2; }
    friend bool operator<=(const std::shared_ptr<date> &d1, const date &d2) { return *d1 <= d2; }

    friend bool operator==(const date &d1, const date &d2) { return d1.m_time_point == d2.m_time_point; }
    friend bool operator==(const std::shared_ptr<date> &d1, const std::shared_ptr<date> &d2) { return *d1 == *d2; }
    friend bool operator==(const date &d1, const std::shared_ptr<date> &d2) { return d1 == *d2; }
    friend bool operator==(const std::shared_ptr<date> &d1, const date &d2) { return *d1 == d2; }

    friend bool operator!=(const date &d1, const date &d2) { return d1.m_time_point != d2.m_time_point; }
    friend bool operator!=(const std::shared_ptr<date> &d1, const std::shared_ptr<date> &d2) { return *d1 != *d2; }
    friend bool operator!=(const date &d1, const std::shared_ptr<date> &d2) { return d1 != *d2; }
    friend bool operator!=(const std::shared_ptr<date> &d1, const date &d2) { return *d1 != d2; }

  private:
    static int calc_max_day(int year, int mon);

    static void calc_tm(struct tm *tm_);

    static std::string replace_format(const char *format_string = "yyyy-MM-dd HH:mm:ss");

  private:
    _time_point m_time_point;
  };
}

#endif //CLANGTOOLS_DATE_HPP
