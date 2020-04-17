
find_package(PkgConfig)
IF (APPLE)
    set(ncurses_pkg_dir "/usr/local/opt/ncurses/lib/pkgconfig")
    set(openssl_pkg_dir "/usr/local/opt/openssl/lib/pkgconfig")
    set(ENV{PKG_CONFIG_PATH} "${openssl_pkg_dir}:${ncurses_pkg_dir}:$ENV{PKG_CONFIG_PATH}")
endif ()
option(ENABLE_OPENSSL "option for OPENSSL" OFF)
if (ENABLE_OPENSSL)
    pkg_search_module(OPENSSL openssl)
    if (OPENSSL_FOUND)
        add_definitions(-DENABLE_OPENSSL)
        include_directories(${OPENSSL_INCLUDE_DIRS})
        if ("${Tools_Other_Project}" STREQUAL "ON")
            message(STATUS "OPENSSL library status:")
            message(STATUS "    ${OPENSSL_VERSION}")
            message(STATUS "    libraries: ${OPENSSL_LIBS}")
            message(STATUS "    libraries: ${OPENSSL_LIBRARIES}")
            message(STATUS "    lib_dir: ${OPENSSL_LIBRARY_DIRS}")
            message(STATUS "    include path: ${OPENSSL_INCLUDE_DIRS}")
        endif ()
        include_directories(${OPENSSL_INCLUDE_DIRS})
        link_directories(${OPENSSL_LIBRARY_DIRS})
    endif ()
endif (ENABLE_OPENSSL)

option(ENABLE_X11 "option for X11" OFF)
if (ENABLE_X11)
    find_package(X11)
    if (X11_FOUND)
        add_definitions(-DENABLE_X11)
        include_directories(${X11_INCLUDE_DIRS})
        if ("${Tools_Other_Project}" STREQUAL "ON")
            message(STATUS "X11 library status:")
            message(STATUS "    ${X11_VERSION}")
            message(STATUS "    libraries: ${X11_LIBRARIES}")
            message(STATUS "    lib_dir: ${X11_LIBRARY_DIRS}")
            message(STATUS "    include path: ${X11_INCLUDE_DIRS}")
        endif ()
        include_directories(${X11_INCLUDE_DIRS})
        link_directories(${X11_LIBRARY_DIRS})
    endif ()
endif (ENABLE_X11)

option(ENABLE_ICONV "option for ICONV" OFF)
if (ENABLE_ICONV)
    pkg_search_module(Iconv iconv)
    if (Iconv_FOUND)
    else ()
        find_package(Iconv)
    endif ()
    if (Iconv_FOUND)
    else ()

        include(ExternalProject)

        if (WIN32)
            if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 16 2019")
                if (${CMAKE_GENERATOR_PLATFORM} STREQUAL "")
                    set(CMAKE_GENERATOR_PLATFORM WIN64)
                endif ()
                set(G_CMAKE_GENERATOR_PLATFORM
                        -G "${CMAKE_GENERATOR}" -A "${CMAKE_GENERATOR_PLATFORM}")
            elseif ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "")
                set(G_CMAKE_GENERATOR_PLATFORM
                        -G "${CMAKE_GENERATOR}")
            else ()
                set(G_CMAKE_GENERATOR_PLATFORM
                        -G "${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM}")
            endif ()
            # ExternalProject_Add(Iconv
            #        URL https://github.com/curl/curl/archive/curl-7_67_0.tar.gz
            #        URL_MD5 "7d2a800b952942bb2880efb00cfd524c"
            #        CONFIGURE_COMMAND cmake -DCMAKE_BUILD_TYPE=RELEASE ${G_CMAKE_GENERATOR_PLATFORM} -DCMAKE_USER_MAKE_RULES_OVERRIDE=${ToolsCmakePath}/MSVC.cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DSTACK_DIRECTION=-1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
            #        PREFIX ${CMAKE_BINARY_DIR}/dependencies
            #        INSTALL_DIR ${INSTALL_DIR}
            #        BUILD_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/curl-build"
            #        INSTALL_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/curl-build" --target install
            #        )
        else ()
            ExternalProject_Add(Iconv
                    URL https://github.com/ClangTools/clangTools/releases/download/libiconv-1.16/libiconv-1.16.tar.gz
                    URL_MD5 "7d2a800b952942bb2880efb00cfd524c"
                    # CONFIGURE_COMMAND echo CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} <SOURCE_DIR>/configure --prefix=${CMAKE_BINARY_DIR}/dependencies --enable-static=yes --enable-shared=no
                    CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} <SOURCE_DIR>/configure --prefix=${CMAKE_BINARY_DIR}/dependencies --enable-static=yes --enable-shared=no
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    PREFIX ${CMAKE_BINARY_DIR}/dependencies
                    INSTALL_DIR ${INSTALL_DIR}
                    BUILD_COMMAND ${MAKE}
                    )
        endif ()
        set(Iconv_FOUND ON)
        set(Iconv_LIB_DIR "${CMAKE_BINARY_DIR}/dependencies/lib")
        set(prefix "lib")
        if (WIN32)
            # set(suffix "-d.lib")
            set(suffix ".lib")
            set(Iconv_LIBRARIES
                    "${Iconv_LIB_DIR}/${prefix}charset${suffix}"
                    "${Iconv_LIB_DIR}/${prefix}iconv${suffix}"
                    )
        else ()
            set(suffix ".a")
            set(Iconv_LIBRARIES
                    "${Iconv_LIB_DIR}/${prefix}charset${suffix}"
                    "${Iconv_LIB_DIR}/${prefix}iconv${suffix}"
                    )
        endif ()
        include_directories(${CMAKE_BINARY_DIR}/dependencies/include/)
        add_definitions(-DBUILDING_LIBICONV)
        add_dependencies(${libTools_LIBRARIES} Iconv)
    endif ()
    if (Iconv_FOUND)
        set(ENABLE_ICONV ON)
        add_definitions(-DENABLE_ICONV)
        if ("${Tools_Other_Project}" STREQUAL "ON")
            message(STATUS "Iconv library status:")
            message(STATUS "    version: ${Iconv_VERSION}")
            message(STATUS "    libraries: ${Iconv_LIBRARY_NAMES}")
            message(STATUS "    libraries: ${Iconv_LIBRARIES}")
            message(STATUS "    lib_dir: ${Iconv_LIB_DIR}")
            message(STATUS "    lib_dir: ${Iconv_LIBRARY_DIRS}")
            message(STATUS "    include path: ${Iconv_INCLUDE_DIRS}")
        endif ()
        link_directories(${Iconv_LIB_DIR})
    endif ()
endif (ENABLE_ICONV)

option(ENABLE_GTK3 "option for GTK+-3" OFF)
if (ENABLE_GTK3)
    add_definitions(-DENABLE_GTK3)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    if ("${GTK3_LINK_LIBRARIES}" STREQUAL "")
    else ()
        set(GTK3_LIBRARIES ${GTK3_LINK_LIBRARIES})
    endif ()
    if (GTK3_FOUND)
        add_definitions(-DENABLE_GTK3)
        if ("${Tools_Other_Project}" STREQUAL "ON")
            message(STATUS "GTK3 library status:")
            message(STATUS "    ${GTK3_VERSION}")
            message(STATUS "    libraries: ${GTK3_LIBRARIES}")
            message(STATUS "    lib_dir: ${GTK3_LIBRARY_DIRS}")
            message(STATUS "    include path: ${GTK3_INCLUDE_DIRS}")
        endif ()
        include_directories(${GTK3_INCLUDE_DIRS})
        link_directories(${GTK3_LIBRARY_DIRS})
    endif ()
endif (ENABLE_GTK3)

option(ENABLE_LIBUSB1 "option for libusb-1.0 tool" OFF)
if (ENABLE_LIBUSB1)
    # find_package(libusb-1.0)
    pkg_search_module(LIBUSB_1 libusb-1.0)
    if (LIBUSB_1_FOUND)
        add_definitions(-DLIBICONV_PLUG)
        if ("${Tools_Other_Project}" STREQUAL "ON")
            message(STATUS "LIBUSB_1 library status:")
            message(STATUS "    ${LIBUSB_1_VERSION}")
            message(STATUS "    libraries: ${LIBUSB_1_LIBRARIES}")
            message(STATUS "    lib_dir: ${LIBUSB_1_LIBRARY_DIRS}")
            message(STATUS "    include path: ${LIBUSB_1_INCLUDE_DIRS}")
        endif ()
        include_directories(${LIBUSB_1_INCLUDE_DIRS})
        link_directories(${LIBUSB_1_LIBRARY_DIRS})
    endif ()
endif ()

option(ENABLE_OPENCV "option for OpenCV" OFF)
if (ENABLE_OPENCV)
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
    option(ENABLE_FREETYPE "option for Freetype" OFF)
    if (ENABLE_FREETYPE)
        find_package(Freetype REQUIRED)
        if (Freetype_FOUND)
            add_definitions(-DENABLE_FREETYPE)
            if ("${Tools_Other_Project}" STREQUAL "ON")
                message(STATUS "Freetype library status:")
                message(STATUS "    version: ${FREETYPE_VERSION}")
                message(STATUS "    libraries: ${FREETYPE_LIBS}")
                message(STATUS "    libraries: ${FREETYPE_LIBRARIES}")
                message(STATUS "    lib_dir: ${FREETYPE_LIB_DIR}")
                message(STATUS "    include path: ${FREETYPE_INCLUDE_DIRS}")
            endif ()
            link_directories(${FREETYPE_LIB_DIR})
            include_directories(
                    ${FREETYPE_INCLUDE_DIRS}
            )
        endif ()
    endif (ENABLE_FREETYPE)
endif (ENABLE_OPENCV)


option(ENABLE_CURL "option for CURL" OFF)
if (ENABLE_CURL)
    find_package(CURL)
    IF (CURL_FOUND)
        INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
    ELSE (CURL_FOUND)
        include(ExternalProject)

        if (WIN32)
            if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 16 2019")
                if (${CMAKE_GENERATOR_PLATFORM} STREQUAL "")
                    set(CMAKE_GENERATOR_PLATFORM WIN64)
                endif ()
                set(G_CMAKE_GENERATOR_PLATFORM
                        -G "${CMAKE_GENERATOR}" -A "${CMAKE_GENERATOR_PLATFORM}")
            elseif ("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "")
                set(G_CMAKE_GENERATOR_PLATFORM
                        -G "${CMAKE_GENERATOR}")
            else ()
                set(G_CMAKE_GENERATOR_PLATFORM
                        -G "${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM}")
            endif ()
            ExternalProject_Add(CURL
                    URL https://github.com/curl/curl/archive/curl-7_67_0.tar.gz
                    URL_MD5 "90b6c61cf3a96a11494deae2f1b3fa92"
                    CONFIGURE_COMMAND cmake -DCMAKE_BUILD_TYPE=RELEASE ${G_CMAKE_GENERATOR_PLATFORM} -DCMAKE_USER_MAKE_RULES_OVERRIDE=${ToolsCmakePath}/MSVC.cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DSTACK_DIRECTION=-1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                    PREFIX ${CMAKE_BINARY_DIR}/dependencies
                    INSTALL_DIR ${INSTALL_DIR}
                    BUILD_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/curl-build"
                    INSTALL_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/curl-build" --target install
                    )
        else ()
            ExternalProject_Add(CURL
                    URL https://github.com/curl/curl/archive/curl-7_67_0.tar.gz
                    URL_MD5 "90b6c61cf3a96a11494deae2f1b3fa92"
                    CONFIGURE_COMMAND cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DSTACK_DIRECTION=-1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                    PREFIX ${CMAKE_BINARY_DIR}/dependencies
                    INSTALL_DIR ${INSTALL_DIR}
                    BUILD_COMMAND ${MAKE}
                    )
        endif ()
        set(CURL_FOUND ON)
        set(CURL_LIB_DIR "${CMAKE_BINARY_DIR}/dependencies/lib")
        set(prefix "lib")
        if (WIN32)
            #            set(suffix "-d.lib")
            set(suffix ".lib")
            set(CURL_LIBRARIES
                    "${CURL_LIB_DIR}/${prefix}curl${suffix}" ws2_32.lib winmm.lib wldap32.lib)
        else ()
            set(suffix ".a")
        endif ()
        link_directories(${CURL_LIB_DIR})
        include_directories(${CMAKE_BINARY_DIR}/dependencies/include/)
        add_definitions(-DBUILDING_LIBCURL -DHTTP_ONLY)

        add_dependencies(${libTools_LIBRARIES} CURL)
        target_link_libraries(${libTools_LIBRARIES} ${CURL_LIBRARIES})
    ENDIF (CURL_FOUND)
    if (CURL_FOUND)
        if ("${Tools_Other_Project}" STREQUAL "ON")
            message(STATUS "CURL library status:")
            message(STATUS "    version: ${CURL_VERSION}")
            message(STATUS "    libraries: ${CURL_LIBS}")
            message(STATUS "    libraries: ${CURL_LIBRARIES}")
            message(STATUS "    lib_dir: ${CURL_LIB_DIR}")
            message(STATUS "    include path: ${CURL_INCLUDE_DIRS}")
        endif ()
        add_definitions(-DENABLE_CURL=ON)
    else ()
        set(${CURL_LIBRARIES} "")
    endif ()
ENDIF (ENABLE_CURL)


option(ENABLE_NCURSES "option for NCURSES" OFF)
if (ENABLE_NCURSES)
    pkg_search_module(NCURSES ncurses)
    if (NCURSES_FOUND)
        add_definitions(-DENABLE_NCURSES)
        include_directories(${NCURSES_INCLUDE_DIRS})

        if ("${Tools_Other_Project}" STREQUAL "ON")
            message(STATUS "NCURSES library status:")
            message(STATUS "    ${NCURSES_VERSION}")
            message(STATUS "    libraries: ${NCURSES_LIBRARIES}")
            message(STATUS "    libraries: ${NCURSES_LINK_LIBRARIES}")
            message(STATUS "    lib_dir: ${NCURSES_LIBRARY_DIRS}")
            message(STATUS "    include path: ${NCURSES_INCLUDE_DIRS}")
        endif ()
        if ("${NCURSES_LINK_LIBRARIES}" STREQUAL "")
        else ()
            set(NCURSES_LIBRARIES ${NCURSES_LINK_LIBRARIES})
        endif ()
        set(NCURSES_LIBRARIES ${NCURSES_LIBRARIES} menu)
        include_directories(${NCURSES_INCLUDE_DIRS})
        link_directories(${NCURSES_LIBRARY_DIRS})
    endif ()
endif (ENABLE_NCURSES)

