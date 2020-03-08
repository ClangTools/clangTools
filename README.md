# clangTools 使用教程

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
[xml](src/xml)
[i2c_tool](src/i2c_tool)
[libusb_1_tool](src/libusb_1_tool)
[opencv_tool](src/opencv_tool)

### i2c_tool

为 `Linux` 接口，用于`Linux`以及`嵌入式设备`。需要添加参数 `-DENABLE_I2C=ON` 开启。

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
