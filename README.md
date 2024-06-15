# clangTools 使用教程

## 编译状态 build status

build status : ![C/C++ CI](https://github.com/ClangTools/clangTools/workflows/C/C++%20CI/badge.svg)

## 支持

如果有任何需要，可以联系我：

>
> kekxv1@163.com
>
> caesar@kekxv.com
>

If you need anything, please contact me :

>
> kekxv1@163.com
>
> caesar@kekxv.com
>

## 支持功能

[Base64](src/Base64),
[ConfigTool](src/ConfigTool),
[http](src/http),
[JSON](src/JSON),
[logger](src/logger),
[memory_share](src/memory_share),
[openssl](src/openssl),
[Pipe](src/Pipe),
[poll_tool](src/poll_tool),
[Popen](src/Popen),
[SHA1](src/SHA1),
[socket](src/socket),
[subprocess](src/subprocess),
[thread_pool](src/thread_pool),
[date](src/date),
[xml](src/xml)
[i2c_tool](src/i2c_tool)
[libusb_1_tool](src/libusb_1_tool)
[opencv_tool](src/opencv_tool)
[Hzk_tool](src/Hzk_tool)
[plthook](src/plthook)([here](https://github.com/kubo/plthook))
[BMP-C](src/BmpTool)([here](https://github.com/zhongcheng0519/BMP-C))
[littlefs](src/littlefs)([here](https://github.com/ARMmbed/littlefs))

### date

一个简单的时间格式库

使用方式：[testDate.cpp](Example%2FtestDate.cpp)

```c++
//
// Created by caesar kekxv on 2024/6/14.
//
#include <date.hpp>
#include <iostream>
#include <regex>

int main(int argc, const char *argv[]) {
  const auto date1 = clangTools::date::parse(1718372061);
  std::cout << __LINE__ << ":" << date1->format() << std::endl;
  const auto date2 = clangTools::date::parseMicroseconds(1718372061000);
  std::cout << __LINE__ << ":" << date2->format() << std::endl;
  const auto date3 = clangTools::date::parse(2024, 06 - 1, 14);
  std::cout << __LINE__ << ":" << date3->format() << std::endl;
  const auto date4 = clangTools::date::parse(2024, 06 - 1, 32, 8, 59);
  std::cout << " " << date4->format("yyyy-MM-dd HH:mm:ss");
  std::cout << "\t" << "yyyy-MM-dd HH:mm:ss" << std::endl;
  std::cout << " " << *date4;
  std::cout << "\t" << "yyyy-MM-dd HH:mm:ss" << std::endl;
  std::cout << " " << date4->format("yyyy+1-MM-1-dd HH:mm:ss");
  std::cout << "\t" << "yyyy+1-MM-1-dd HH:mm:ss" << std::endl;
  std::cout << " " << date4->format("yyyy--06-dd HH:mm:ss");
  std::cout << "\t" << "yyyy--06-dd HH:mm:ss" << std::endl;
  std::cout << " " << date4->clone("yyyy-1-MM+1-dd HH:mm:ss")->format("yyyy-MM-dd HH:mm:ss");
  std::cout << "\t" << "yyyy-1-MM+1-dd HH:mm:ss" << "_" << "yyyy-MM-dd HH:mm:ss" << std::endl;
  std::cout << " " << date4->format("yyyy--07--02 HH:mm:ss");
  std::cout << "\t" << "yyyy--07--02 HH:mm:ss" << std::endl;
  std::cout << "\t" << date1 << std::endl;
  std::cout << "\t" << date2 << std::endl;
  std::cout << "\t" << (date1 == date2) << std::endl;
  return 0;
}

```

输出：

```log
10:2024-06-14 21:34:21
12:2024-06-14 21:34:21
14:2024-06-14 00:00:00
 2024-07-02 08:59:00	yyyy-MM-dd HH:mm:ss
 2024-07-02 08:59:00	yyyy-MM-dd HH:mm:ss
 2025-06-02 08:59:00	yyyy+1-MM-1-dd HH:mm:ss
 2024-06-02 08:59:00	yyyy--06-dd HH:mm:ss
 2023-08-02 08:59:00	yyyy-1-MM+1-dd HH:mm:ss_yyyy-MM-dd HH:mm:ss
 2024-07-02 08:59:00	yyyy--07--02 HH:mm:ss
	2024-06-14 21:34:21
	2024-06-14 21:34:21
	1
```

### i2c_tool

为 `Linux` 接口，用于`Linux`以及`嵌入式设备`。需要添加参数 `-DENABLE_I2C=ON` 开启。

### spi_tool

为 `Linux` 接口，用于`Linux`以及`嵌入式设备`。需要添加参数 `-DENABLE_SPI=ON` 开启。

## 引入方式

```cmake
# 添加 clangTools 模块
add_subdirectory(clangTools)
# 设置模块目录
set(libTools_DIR "${CMAKE_CURRENT_SOURCE_DIR}/clangTools")
# 引入 clangTools 模块变量
include(clangTools/libTools.cmake)
# 设置 Windows 下使用 MT 模式(非MD模式)
include(clangTools/cmake/MSVC.cmake)
# 加入头文件目录
include_directories(${libTools_INCLUDE_DIR})
# 链接 clangTools 模块变量 ： ${libTools_LIBRARIES}


option(ENABLE_FILENAME "option for FILENAME" ON)
if (ENABLE_FILENAME)
    #重新定义当前目标的源文件的 __FILENAME__ 宏
    redefine_file_macro(目标)
endif ()
```

## bazel 支持

本项目部分支持 `bazel`。

build status : ![C/C++ CI](https://github.com/ClangTools/clangTools/workflows/C/C++%20CI/badge.svg)

## demo

### clock oled

编译树莓派 时钟 OLED 程序可以使用以下参数：

```shell script
cmake -DENABLE_CURL=ON -DENABLE_OPENCV=ON -DENABLE_HZK=ON -DENABLE_I2C=ON -DENABLE_SPI=ON -DENABLE_OLED=ON -DENABLE_FREETYPE=ON -DENABLE_ICONV=ON -DENABLE_OLED_TOOL=ON <cmake-source-dir>
make clock_oled
```

将会在 `ssd1306 oled (128x64)` 屏幕上显示`日期`、`温度`、`天气`、`时间`、`IP地址`。

## 致谢

非常感谢 [`JetBrains`](https://account.jetbrains.com/) 为本项目提供开源项目授权。

Thank you [`JetBrains`](https://account.jetbrains.com/) for providing open source project authorization for this
project.