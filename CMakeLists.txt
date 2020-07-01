cmake_minimum_required(VERSION 3.0)

if ("${PROJECT_NAME}" STREQUAL "Project")
    set(Tools_Other_Project ON)
elseif ("${PROJECT_NAME}" STREQUAL "")
    set(Tools_Other_Project ON)
endif ()
project(Tools)
if (APPLE)
else ()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++ -static-libgcc -Wl,-rpath=.") # -static
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libstdc++ -static-libgcc -Wl,-rpath=.") # -static
endif ()

include(cmake/MSVC.cmake)
include(cmake/setCPP11.cmake)

set(libTools_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

FILE(GLOB ToolsCmakePath "${libTools_DIR}/cmake")

set(CMAKE_CXX_STANDARD 14)

set(libTools_LIBRARIES Tools)
set_property(GLOBAL PROPERTY libTools_LIBRARIES ${libTools_LIBRARIES})
if (WIN32)
    set(dl_LIBRARIES ws2_32.lib)
else ()
    find_library(dl dl)
    set(dl_LIBRARIES dl pthread)
endif ()

option(ANDROID_SO "option for ANDROID_SO" OFF)
if (ANDROID_SO)
    add_definitions(-DANDROID_SO)
endif ()


FILE(GLOB srcSourceHeader "src/*")
list(FILTER srcSourceHeader EXCLUDE REGEX ".*libVersion\\.rc")


option(ENABLE_I2C "option for I2C tool" OFF)
if (ENABLE_I2C)
    add_definitions(-DENABLE_I2C)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*i2c_tool")
endif ()
option(ENABLE_HZK "option for Hzk tool" OFF)
if (ENABLE_HZK)
    add_definitions(-DENABLE_HZK)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*Hzk_tool")
endif ()
option(ENABLE_SPI "option for SPI tool" OFF)
if (ENABLE_SPI)
    add_definitions(-DENABLE_SPI)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*spi_tool")
endif ()
option(ENABLE_C_VECTOR "option for C_VECTOR" ON)
if (ENABLE_C_VECTOR)
    add_definitions(-DENABLE_C_VECTOR)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*C_VECTOR")
endif ()

option(ENABLE_OpenJPEG "option for OpenJPEG" OFF)
if (ENABLE_OpenJPEG)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*openjpeg")
endif ()
option(ENABLE_LIBUSB1 "option for libusb-1.0 tool" OFF)
if (ENABLE_LIBUSB1)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*libusb_1_tool")
endif ()
option(ENABLE_OLED_TOOL "option for oled_tool tool" OFF)
if (ENABLE_OLED_TOOL)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*oled_tool")
endif ()
option(ENABLE_OPENCV "option for OpenCV" OFF)
if (ENABLE_OPENCV)
    if (UNIX)
        add_definitions(-DCV__ENABLE_C_API_CTORS)
    endif ()
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*freetype_tool")
endif ()
option(ENABLE_FREETYPE "option for FREETYPE" OFF)
if (ENABLE_FREETYPE)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*freetype_tool")
endif ()
option(ENABLE_NCURSES "option for NCURSES" OFF)
if (ENABLE_NCURSES)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*ncurses_tool")
endif ()
if (WIN32)
else ()
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*_win")
endif ()

include_directories(
#        .
        ./src
        ${srcSourceHeader}
)
set_property(GLOBAL PROPERTY libTools_INCLUDE_DIR ${srcSourceHeader})
set(srcSource "")
list(FILTER srcSourceHeader EXCLUDE REGEX ".*plthook")
foreach (item in ${srcSourceHeader})
    FILE(GLOB subSrcSource "${item}/*")
    list(APPEND srcSource ${subSrcSource})
endforeach ()

option(ENABLE_LIBUSB1 "option for libusb-1.0 tool" OFF)
if (ENABLE_LIBUSB1)
    list(FILTER srcSourceHeader EXCLUDE REGEX ".*libusb_1_tool.*hidapi")
    list(APPEND srcSource "src/libusb_1_tool/hidapi/hid.c")
    include_directories("src/libusb_1_tool/hidapi")
endif ()

if (APPLE)
    list(APPEND srcSource "src/plthook/plthook_osx.c")
elseif (UNIX)
    list(APPEND srcSource "src/plthook/plthook_elf.c")
else ()
    list(APPEND srcSource "src/plthook/plthook_win32.c")
endif ()
if (UNIX)
else ()
    list(FILTER srcSource EXCLUDE REGEX ".*Unix.*")
endif ()

add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
)

if (NOT OPENSSL_FOUND)
    list(REMOVE_ITEM srcSource src/openssl/ssl_common.cpp)
endif ()

add_library(${libTools_LIBRARIES} STATIC
        ${RC_FILES}
        ${srcSource}
        )


include(cmake/find_libs.cmake)
if (OPENSSL_FOUND)
    link_directories(${OPENSSL_LIBRARY_DIRS})
endif ()

option(ENABLE_FILENAME "option for FILENAME" ON)
if (ENABLE_FILENAME)
    include(cmake/redefine_filename.cmake)
endif ()

include_directories(
        ${OpenJPEG_INCLUDE_DIRS}
)

target_compile_definitions(
        ${libTools_LIBRARIES}
        PUBLIC -D_${libTools_LIBRARIES}_HEADER_ #-D_LOGGER_USE_THREAD_POOL_
)

# Mark these variables above as deprecated.
function(__deprecated_var var access)
    if(access STREQUAL "READ_ACCESS")
        message(STATUS "The variable '${var}' is deprecated!  Instead, simply use target_link_libraries(your_app dlib::dlib).  See http://dlib.net/examples/CMakeLists.txt.html for an example.")
    endif()
endfunction()

if (CURL_FOUND)
    target_link_libraries(${libTools_LIBRARIES} ${CURL_LIBRARIES})
endif ()
target_link_libraries(${libTools_LIBRARIES}
        ${OpenCV_LIBS} ${FREETYPE_LIBRARIES} ${LIBUSB_1_LIBRARIES}
        ${dl_LIBRARIES} ${OPENSSL_LIBRARIES} ${X11_LIBRARIES} ${GTK3_LIBRARIES} ${NCURSES_LIBRARIES}
        ${OpenJPEG_LIBRARIES} ${dlib_LIBRARIES} ${libevent_LIBRARIES} ${LIBEVENT_LIBRARIES}
        )
target_link_libraries(${libTools_LIBRARIES} ${Iconv_LIBRARIES})


option(ENABLE_FILENAME "option for FILENAME" ON)
if (ENABLE_FILENAME)
    #重新定义当前目标的源文件的 __FILENAME__ 宏
    redefine_file_macro(${libTools_LIBRARIES})
endif ()

include(cmake/resource.cmake)

option(ENABLE_OPENCV_CASCADES "option for OPENCV_CASCADES" OFF)
if (ENABLE_OPENCV_CASCADES)
    add_dependencies(${libTools_LIBRARIES} UnpackingOpenCV_Cascades)
endif (ENABLE_OPENCV_CASCADES)
option(ENABLE_DLIB "option for dlib" OFF)
if (ENABLE_DLIB)
    add_definitions(-DENABLE_DLIB)
    option(ENABLE_DLIB_FACE_LANDMARKS "option for ENABLE_DLIB_FACE_LANDMARKS" OFF)
    if (ENABLE_DLIB_FACE_LANDMARKS)
        option(ENABLE_DLIB_5_FACE_LANDMARKS "option for dlib Shape_predictor_5_face_landmarks" ON)
        if (ENABLE_DLIB_5_FACE_LANDMARKS)
            add_dependencies(${libTools_LIBRARIES} UnpackingShape_predictor_5_face_landmarks)
        else ()
            add_dependencies(${libTools_LIBRARIES} UnpackingShape_predictor_68_face_landmarks)
        endif ()
    endif ()
endif ()
option(ENABLE_FREETYPE "option for FREETYPE" OFF)
if (ENABLE_FREETYPE)
    add_dependencies(${libTools_LIBRARIES} UnpackingOpenDotFont)
endif (ENABLE_FREETYPE)
option(ENABLE_HZK "option for HZK tool" OFF)
if (ENABLE_HZK)
    add_dependencies(${libTools_LIBRARIES} UnpackingHzkFont)
    add_dependencies(${libTools_LIBRARIES} UnpackingGovJson)
endif (ENABLE_HZK)


include(cmake/example.cmake)
include(cmake/install.cmake)

