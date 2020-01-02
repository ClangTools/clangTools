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


find_package(PkgConfig)
pkg_search_module(OPENSSL openssl)
if (OPENSSL_FOUND)
    add_definitions(-DENABLE_OPENSSL)

    include_directories(${OPENSSL_INCLUDE_DIRS})

    if ("${Tools_Other_Project}" STREQUAL "ON")
        message(STATUS "OPENSSL library status:")
        message(STATUS "    ${OPENSSL_VERSION}")
        message(STATUS "    libraries: ${OPENSSL_LIBRARIES}")
        message(STATUS "    lib_dir: ${OPENSSL_LIBRARY_DIRS}")
        message(STATUS "    include path: ${OPENSSL_INCLUDE_DIRS}")
    endif ()
    include_directories(${OPENSSL_INCLUDE_DIRS})
    link_directories(${OPENSSL_LIBRARY_DIRS})

endif ()

option(ENABLE_FILENAME "option for FILENAME" ON)
if (ENABLE_FILENAME)
    #重新定义当前目标的源文件的 __FILENAME__ 宏
    function(redefine_file_macro targetname)
        #获取当前目标的所有源文件
        get_target_property(source_files "${targetname}" SOURCES)
        #遍历源文件
        foreach (sourcefile ${source_files})
            #获取当前源文件的编译参数
            get_property(defs SOURCE "${sourcefile}"
                    PROPERTY COMPILE_DEFINITIONS)
            #获取当前文件的绝对路径
            get_filename_component(filepath "${sourcefile}" ABSOLUTE)
            # message("CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")
            # message("PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
            # message("_SOURCE_DIR ${_SOURCE_DIR}")
            #将绝对路径中的项目路径替换成空,得到源文件相对于项目路径的相对路径
            string(REPLACE ${CMAKE_SOURCE_DIR}/ "" relpath ${filepath})
            #将我们要加的编译参数( __FILENAME__ 定义)添加到原来的编译参数里面
            list(APPEND defs "__FILENAME__=\"${relpath}\"")
            #重新设置源文件的编译参数
            set_property(
                    SOURCE "${sourcefile}"
                    PROPERTY COMPILE_DEFINITIONS ${defs}
            )
        endforeach ()
    endfunction()
endif ()


# 设置为 v140_xp 编译工具编译
# set(CMAKE_GENERATOR_TOOLSET "v140_xp")
# set(CMAKE_GENERATOR_PLATFORM "Win32")

if (MSVC)
    set(CMAKE_SYSTEM_VERSION 5.1)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            )
    foreach (CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach ()
    # It should be set to 1 for the static MFC library, and 2 for the shared MFC library.
    set(CMAKE_MFC_FLAG 1)
    #add_definitions(-DUNICODE -D_UNICODE)
    add_definitions(-D_AFXDLL)
endif (MSVC)

find_package(OpenCV)
if (OpenCV_FOUND)
    set(ENABLE_OPENCV ON)
    add_definitions(-DENABLE_OPENCV)
    if ("${Tools_Other_Project}" STREQUAL "ON")
        message(STATUS "OpenCV library status:")
        message(STATUS "    version: ${OpenCV_VERSION}")
        message(STATUS "    libraries: ${OpenCV_LIBS}")
        message(STATUS "    libraries: ${OpenCV_LIBRARIES}")
        message(STATUS "    lib_dir: ${OpenCV_LIB_DIR}")
        message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
    endif ()
    link_directories(${OpenCV_DIR})
    include_directories(
            ${OpenCV_INCLUDE_DIRS}
    )
endif ()



include_directories(
        .
        src/
)

aux_source_directory(src srcSource)

add_definitions(
        -D_CRT_SECURE_NO_WARNINGS

)
if (NOT OPENSSL_FOUND)
#    set(RC_FILES
#            ${RC_FILES}
#            src/ssl_common.cpp
#            )
    list(REMOVE_ITEM srcSource src/ssl_common.cpp)
endif()

add_library(${libTools_LIBRARIES} STATIC
        ${RC_FILES}
        ${srcSource}
        )
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
    INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
ELSE(CURL_FOUND)
    include(ExternalProject)
    if (WIN32)
        ExternalProject_Add(curl
                URL https://github.com/curl/curl/archive/curl-7_67_0.tar.gz
                URL_MD5 "90b6c61cf3a96a11494deae2f1b3fa92"
                CONFIGURE_COMMAND cmake -G "Visual Studio 14 2015" -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DSTACK_DIRECTION=-1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                PREFIX ${CMAKE_BINARY_DIR}/dependencies
                INSTALL_DIR ${INSTALL_DIR}
                BUILD_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/curl-build"
                INSTALL_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/curl-build" --target install
                )
        else()
        ExternalProject_Add(curl
                URL https://github.com/curl/curl/archive/curl-7_67_0.tar.gz
                URL_MD5 "90b6c61cf3a96a11494deae2f1b3fa92"
                CONFIGURE_COMMAND cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DSTACK_DIRECTION=-1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                PREFIX ${CMAKE_BINARY_DIR}/dependencies
                INSTALL_DIR ${INSTALL_DIR}
                BUILD_COMMAND ${MAKE}
                )
    endif ()
    set(curl_LIB_DIR "${CMAKE_BINARY_DIR}/dependencies/lib")
    set(prefix "lib")
    if (WIN32)
        set(suffix "-d.lib")
    else ()
        set(suffix ".a")
    endif ()
    set(CURL_LIBRARIES
            "${curl_LIB_DIR}/${prefix}curl${suffix}")

    include_directories(${CMAKE_BINARY_DIR}/dependencies/include/)
    add_dependencies(${libTools_LIBRARIES} curl)
ENDIF(CURL_FOUND)

target_compile_definitions(
        ${libTools_LIBRARIES}
        PUBLIC -D_${libTools_LIBRARIES}_HEADER_ #-D_LOGGER_USE_THREAD_POOL_
)
target_link_libraries(${libTools_LIBRARIES} ${OpenCV_LIBS} ${dl_LIBRARIES} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})

option(clangToolsExample "option for FILENAME" OFF)
if (clangToolsExample OR "${Tools_Other_Project}" STREQUAL "ON")
    add_executable(logger_Demo Example/logger_demo.cpp)
    add_executable(http_Demo Example/http_demo.cpp)
    add_executable(thread_pool_demo Example/thread_pool_demo.cpp)
    add_executable(tinyxml2_demo Example/tinyxml2_demo.cpp)
    add_executable(json_demo Example/json_demo.cpp)
    target_link_libraries(http_Demo ${libTools_LIBRARIES} ${dl_LIBRARIES} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})
    target_link_libraries(logger_Demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(thread_pool_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(tinyxml2_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(json_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})

    option(ENABLE_FILENAME "option for FILENAME" ON)
    if (ENABLE_FILENAME)
        redefine_file_macro(http_Demo)
        redefine_file_macro(logger_Demo)
        redefine_file_macro(tinyxml2_demo)
        redefine_file_macro(thread_pool_demo)
        redefine_file_macro(json_demo)
    endif ()

    install(TARGETS http_Demo logger_Demo thread_pool_demo tinyxml2_demo json_demo
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION bin)
    message("-- install to " ${CMAKE_INSTALL_PREFIX})
endif ()


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


########安装脚本##########
install(TARGETS ${libTools_LIBRARIES}
        RUNTIME DESTINATION lib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

FILE(GLOB_RECURSE include_header src/*.h)
install(FILES
        ${include_header}
        DESTINATION include)

# 复制 lib 代码到 lib 下
install(DIRECTORY Example/ DESTINATION Example
        PATTERN "Example/*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ)
