#include <iostream>
#include <HTTP.h>
#include <logger.h>
#include <json11.hpp>
#include <opencv2/opencv.hpp>
#include <ssd1306.h>
#include <net_tool.h>
#include <string>
#include <Hzk_Font.h>
#include <opencv_tool.h>
#include <thread_pool.h>

using namespace std;
using namespace cv;


const char *_province = "广东";
const char *_cityName = "深圳";
json11::Json weather_data;

/**
 * 更新日期
 * @param pMat
 */
void setDate(Mat *pMat);

/**
 * 更新时间
 * @param img
 */
void setTime(Mat *img);

/**
 * 更新 二维码
 * @param img
 */
void setQR(Mat *img);

/**
 * 获取天气详情
 * @param _city_code
 * @return
 */
json11::Json getWeather(const char *province, const char *cityName, const char *county = nullptr) {
    json11::Json json;
    std::vector<unsigned char> data;
    std::string _province = province == nullptr ? "" : province;
    std::string _cityName = cityName == nullptr ? "" : cityName;
    std::string _county = county == nullptr ? "" : county;
    HTTP http_get("https://wis.qq.com/weather/common?source=xw&weather_type=observe|forecast_24h|tips|air&province=" +
                  _province + "&city=" + _cityName + "&county=" + _county);
    data.clear();
    http_get.timeout = 500;
    auto ret = http_get.send(data);
    if (ret >= 0) {
        data.push_back(0);
        std::string err;
        json = json11::Json::parse((const char *) data.data(), err);
        if (err.empty()) {
            if (json["status"].int_value() == 200)
                json = json["data"];
        }
    }
    return json;
}

/**
 * 上次获取 IP 地址时间
 */
long long last_get_ip_time = 0;
/**
 * 上次获取天气时间
 */
long long last_get_weather_time = 0;
/**
 * 天气锁
 */
std::mutex _weather_mutex;

/**
 * 开始时钟循环
 * @param srcImg
 */
void OledShowNew(Mat *srcImg) {
    ssd1306 ssd1306;
    // ssd1306.clear();
    thread_pool get_weather_task(2);
    std::vector<std::string> ips;
    for (int i = 10000; i > 0; i--) {
        cv::Mat img = srcImg->clone();
        setDate(&img);
        setTime(&img);
        setQR(&img);

        { // ip 地址

            if (last_get_ip_time < logger::get_time_tick()) {
                /**
                 * 没十秒钟更新一次 IP 地址
                 */
                last_get_ip_time = logger::get_time_tick() + 10 * 1000;
                ips.clear();
                net_tool::GetIP(ips);
                ips.emplace_back("blog.kekxv.com");
                if (weather_data["observe"]["humidity"].is_string())
                    ips.emplace_back("潮湿度:" + weather_data["observe"]["humidity"].string_value());
            }
            auto _ips = Hzk_Font::instance()->get(ips[(i / 10) % ips.size()], Hzk_Font::S12);
            int x = 0;
            for (int j = 0; j < _ips.size(); j++) {
                auto _ip = _ips[j];
                cv::Mat roi = img(cv::Rect(x, img.rows - _ip.rows, _ip.cols, _ip.rows));
                _ip.copyTo(roi);
                x += _ips[j].cols;
            }
        }

        { // 天气
            if (last_get_weather_time < logger::get_time_tick()) {
                /**
                 * 每 5 分钟更新一次天气情况
                 */
                last_get_weather_time = logger::get_time_tick() + 5 * 60 * 1000;

                get_weather_task.commit([]() -> void {
                    auto _weather_data = getWeather(_province, _cityName);
                    if (weather_data["observe"].is_object()) {
                        logger::instance()->i(__FILENAME__, __LINE__, "%s %s 气温：%s℃；潮湿：%s；气象：%s",
                                              _province,
                                              _cityName,
                                              weather_data["observe"]["degree"].string_value().c_str(),
                                              weather_data["observe"]["humidity"].string_value().c_str(),
                                              weather_data["observe"]["weather"].string_value().c_str()
                        );

                        std::unique_lock<std::mutex> lock(_weather_mutex);
                        weather_data = _weather_data;
                    }

                });
            }
            if (weather_data["observe"].is_object()) {
                std::unique_lock<std::mutex> lock(_weather_mutex);
                string temp = weather_data["observe"]["degree"].string_value();
                auto temps = Hzk_Font::instance()->get(temp, Hzk_Font::S16);
                auto _temp = Hzk_Font::instance()->get("℃", Hzk_Font::S12)[0];
                int x = 0;
                int width = 0;
                for (auto &temp : temps) {
                    width += temp.cols;
                }
                x = 127 - width - _temp.cols;
                for (auto &temp : temps) {
                    cv::Mat roi = img(cv::Rect(x, ssd1306::GetLineY14(1), temp.cols, temp.rows));
                    temp.copyTo(roi);
                    x += temp.cols;
                }
                cv::Mat roi = img(cv::Rect(127 - _temp.cols, ssd1306::GetLineY14(1), _temp.cols, _temp.rows));
                _temp.copyTo(roi);


                int weather_len = 3;
                int weather_len_rows = 3;
                string weather = weather_data["observe"]["weather"].string_value();
                auto weathers = Hzk_Font::instance()->get(weather, Hzk_Font::S14);
                int weather_index = 0;
                int weather_y = 15 + 15 / 2;
                // logger::instance()->d(__FILENAME__,__LINE__,"weathers.size() : %zu",weathers.size());
                while (weather_index < weathers.size()) {
                    _temp = weathers[weather_index];
                    roi = img(
                            cv::Rect((weather_index % weather_len) * _temp.cols,
                                     weather_y + (weather_index / weather_len) * (_temp.rows + 1),
                                     _temp.cols,
                                     _temp.rows));
                    _temp.copyTo(roi);
                    weather_index++;
                    if (weather_index >= weather_len * weather_len_rows)break;
                }
            }
        }


        /**
         * 更新到oled显示器上
         */
        ssd1306.draw(&img, true);
        usleep(10 * 1000);
        if (i < 2)i = 10000;
    }
}

void setQR(Mat *img) { // 二维码
    cv::Mat qrImg = opencv_tool::CreateQrCode("http://blog.kekxv.com", 1);
    cv::Mat roi = (*img)(
            cv::Rect(127 - qrImg.size().width, ssd1306::GetLineY14(4) - qrImg.size().height / 2, qrImg.cols,
                     qrImg.rows));
    qrImg.copyTo(roi);
}

void setTime(Mat *img) {
    int x = 4;
    int y = 0;
    int width = 0;
    int height = 0;
    // auto times = Hzk_Font::instance()->get(logger::GetTime("%H:%M:%S"), Hzk_Font::S16);
    string str_time = logger::GetTime("%H:%M %S");
    auto str_index = str_time.find(' ');
    auto times = Hzk_Font::instance()->get(str_time.substr(0, str_index), Hzk_Font::S24);

    auto times_s = Hzk_Font::instance()->get(str_time.substr(str_index + 1), Hzk_Font::S12);
    for (auto &time : times) {
        width += time.cols - 1;
        height = height > time.rows ? height : time.rows;
    }
    width = (128 - width);
    if (width < 0)return;
    height = (64 - height);
    if (height < 0)return;
    x += width / 2;
    y += height / 2;
    for (auto &time : times) {
        auto _time = time;
        cv::Mat roi = (*img)(cv::Rect(x, y, _time.cols, _time.rows));
        _time.copyTo(roi);
        x += time.cols - 1;
    }
    for (auto &time : times_s) {
        auto _time = time;
        cv::Mat roi = (*img)(cv::Rect(x, y, _time.cols, _time.rows));
        _time.copyTo(roi);
        x += time.cols;
    }

}

void setDate(Mat *pMat) {
    int x = 0;
    auto dates = Hzk_Font::instance()->get(logger::GetTime("%Y/%m/%d"), Hzk_Font::S12);
    for (auto &date : dates) {
        auto _date = date;
        cv::Mat roi = (*pMat)(cv::Rect(x, ssd1306::GetLineY14(1), _date.cols, _date.rows));
        _date.copyTo(roi);
        x += date.cols + 1;
    }
}

int main() {
    logger::instance()->init_default();
    logger::instance()->i(__FILENAME__, __LINE__, "start");
    cv::Mat srcImg(64, 128, CV_8UC3, cv::Scalar(255, 255, 255)); // create a black background

    weather_data = getWeather(_province, _cityName);

    if (weather_data["observe"].is_object())
        logger::instance()->i(__FILENAME__, __LINE__, "%s %s 气温：%s℃；潮湿：%s；气象：%s",
                              _province,
                              _cityName,
                              weather_data["observe"]["degree"].string_value().c_str(),
                              weather_data["observe"]["humidity"].string_value().c_str(),
                              weather_data["observe"]["weather"].string_value().c_str()
        );


    OledShowNew(&srcImg);

    logger::instance()->i(__FILENAME__, __LINE__, "end");
    return 0;
}

