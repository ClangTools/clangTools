cmake_minimum_required(VERSION 3.0)

if ("${PROJECT_NAME}" STREQUAL "Project")
    set(Tools_Other_Project ON)
elseif ("${PROJECT_NAME}" STREQUAL "")
    set(Tools_Other_Project ON)
endif ()
project(Tools)

set(CMAKE_CXX_STANDARD 14)

set(libTools_LIBRARIES Tools)
set_property(GLOBAL PROPERTY libTools_LIBRARIES ${libTools_LIBRARIES})

if (WIN32)
    set(dl_LIBRARIES ws2_32.lib)
else ()
    find_library(dl dl)
    set(dl_LIBRARIES dl)
endif ()


include(cmake/find_libs.cmake)

option(ENABLE_FILENAME "option for FILENAME" ON)
if (ENABLE_FILENAME)
    include(cmake/redefine_filename.cmake)
endif ()

include(cmake/MSVC.cmake)


FILE(GLOB srcSourceHeader "src/*")

include_directories(
        .
        ${srcSourceHeader}
)
set_property(GLOBAL PROPERTY libTools_INCLUDE_DIR ${srcSourceHeader})

set(srcSource "")
foreach (item in ${srcSourceHeader})
    FILE(GLOB subSrcSource "${item}/*")
    list(APPEND srcSource ${subSrcSource})
endforeach ()

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

target_compile_definitions(
        ${libTools_LIBRARIES}
        PUBLIC -D_${libTools_LIBRARIES}_HEADER_ #-D_LOGGER_USE_THREAD_POOL_
)


target_link_libraries(${libTools_LIBRARIES} ${OpenCV_LIBS} ${dl_LIBRARIES} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})


add_custom_command(TARGET ${libTools_LIBRARIES} POST_BUILD        # Adds a post-build event to MyTest
        COMMAND ${CMAKE_COMMAND} -E copy_directory   # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/Depend"      # <--this is in-file
        "${CMAKE_CURRENT_BINARY_DIR}"
        )                 # <--this is out-file path


option(ENABLE_FILENAME "option for FILENAME" ON)
if (ENABLE_FILENAME)
    #重新定义当前目标的源文件的 __FILENAME__ 宏
    redefine_file_macro(${libTools_LIBRARIES})
endif ()

include(cmake/example.cmake)
include(cmake/install.cmake)
