
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
        include(CheckSymbolExists)
        # Check for macro SEEK_SET
        check_symbol_exists(iconv_open "iconv.h" HAVE_iconv_open)

        if (HAVE_iconv_open)
            set(ENABLE_ICONV ON)
            add_definitions(-DENABLE_ICONV)
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
    else ()
        pkg_search_module(LIBUSB_1 libusb)
    endif ()
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


option(ENABLE_OpenJPEG "option for OpenJPEG" OFF)
if (ENABLE_OpenJPEG)
    #    find_package(OpenJPEG)
    #    IF (OpenJPEG_FOUND)
    #        INCLUDE_DIRECTORIES(${OpenJPEG_INCLUDE_DIR})
    #    ELSE (OpenJPEG_FOUND)
    include(ExternalProject)

    if (libTools_LIBRARIES_CMAKE)
    else ()
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
            ExternalProject_Add(OpenJPEG
                    URL https://github.com/ClangTools/openjpeg/archive/336835624ca4a040a9780beddbbb52a45916f1eb.zip
                    URL_MD5 "00cfd5f465d4621bdc5a71bbea6f7662"
                    CONFIGURE_COMMAND cmake -DCMAKE_BUILD_TYPE=RELEASE ${G_CMAKE_GENERATOR_PLATFORM} -DCMAKE_USER_MAKE_RULES_OVERRIDE=${ToolsCmakePath}/MSVC.cmake -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                    PREFIX ${CMAKE_BINARY_DIR}/dependencies
                    INSTALL_DIR ${INSTALL_DIR}
                    BUILD_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/OpenJPEG-build"
                    INSTALL_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/OpenJPEG-build" --target install
                    )
        else ()
            ExternalProject_Add(OpenJPEG
                    URL https://github.com/ClangTools/openjpeg/archive/336835624ca4a040a9780beddbbb52a45916f1eb.zip
                    URL_MD5 "00cfd5f465d4621bdc5a71bbea6f7662"
                    CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                    PREFIX ${CMAKE_BINARY_DIR}/dependencies
                    INSTALL_DIR ${INSTALL_DIR}
                    BUILD_COMMAND ${MAKE}
                    )
        endif ()
    endif ()

    set(OpenJPEG_FOUND ON)
    set(OpenJPEG_LIB_DIR "${CMAKE_BINARY_DIR}/dependencies/lib")
    set(prefix "lib")
    if (WIN32)
        #            set(suffix "-d.lib")
        set(suffix ".lib")
        set(OpenJPEG_LIBRARIES
                "${OpenJPEG_LIB_DIR}/${prefix}openjp2${suffix}" ws2_32.lib winmm.lib wldap32.lib)
    else ()
        set(suffix ".a")
        set(OpenJPEG_LIBRARIES "${OpenJPEG_LIB_DIR}/${prefix}openjp2${suffix}")
    endif ()
    set(OpenJPEG_LIBS ${OpenJPEG_LIBRARIES})
    link_directories(${OpenJPEG_LIB_DIR})
    set(OpenJPEG_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/dependencies/include/)
    include_directories(${OpenJPEG_INCLUDE_DIRS})
    add_definitions(-DBUILDING_LIBOpenJPEG)

    add_dependencies(${libTools_LIBRARIES} OpenJPEG)
    if (libTools_LIBRARIES_CMAKE)
    else ()
        target_link_libraries(${libTools_LIBRARIES} ${OpenJPEG_LIBRARIES})
    endif ()
    #    ENDIF (OpenJPEG_FOUND)
    if (OpenJPEG_FOUND)
        if ("${Tools_Other_Project}" STREQUAL "ON")
            message(STATUS "OpenJPEG library status:")
            message(STATUS "    version: ${OpenJPEG_VERSION}")
            message(STATUS "    libraries: ${OpenJPEG_LIBS}")
            message(STATUS "    libraries: ${OpenJPEG_LIBRARIES}")
            message(STATUS "    lib_dir: ${OpenJPEG_LIB_DIR}")
            message(STATUS "    include path: ${OpenJPEG_INCLUDE_DIRS}")
        endif ()
        add_definitions(-DENABLE_OpenJPEG=ON)
    else ()
        set(${OpenJPEG_LIBRARIES} "")
    endif ()
ENDIF (ENABLE_OpenJPEG)

option(ENABLE_DLIB "option for dlib" OFF)
if (ENABLE_DLIB)
    find_package(dlib QUIET)
    IF (dlib_FOUND)
        INCLUDE_DIRECTORIES(${dlib_INCLUDE_DIR})
    ELSE (dlib_FOUND)
        include(ExternalProject)
        set(DLIB_BUILD_TYPE " -DDLIB_USE_BLAS=OFF -DDLIB_USE_LAPACK=OFF")
        #        option(DLIB_NO_GUI_SUPPORT "option for dlib DLIB_NO_GUI_SUPPORT" ON)
        #        set(DLIB_BUILD_TYPE " -DDLIB_NO_GUI_SUPPORT=${DLIB_NO_GUI_SUPPORT} ${DLIB_BUILD_TYPE}")
        message(STATUS DLIB_BUILD_TYPE ${DLIB_BUILD_TYPE})

        if (libTools_LIBRARIES_CMAKE)
        else ()
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
                ExternalProject_Add(dlib
                        URL https://github.com/davisking/dlib/archive/v19.19.tar.gz
                        URL_MD5 "94ec18c4f31c6c3b9af15306af1867ee"
                        CONFIGURE_COMMAND cmake -DCMAKE_BUILD_TYPE=RELEASE ${G_CMAKE_GENERATOR_PLATFORM} -DCMAKE_USER_MAKE_RULES_OVERRIDE=${ToolsCmakePath}/MSVC.cmake ${DLIB_BUILD_TYPE} DDLIB_USE_BLAS=OFF -DDLIB_USE_LAPACK=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                        PREFIX ${CMAKE_BINARY_DIR}/dependencies
                        INSTALL_DIR ${INSTALL_DIR}
                        BUILD_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/dlib-build"
                        INSTALL_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/dlib-build" --target install
                        )
            else ()
                ExternalProject_Add(dlib
                        URL https://github.com/davisking/dlib/archive/v19.19.tar.gz
                        URL_MD5 "94ec18c4f31c6c3b9af15306af1867ee"
                        CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER} cmake -DCMAKE_BUILD_TYPE=RELEASE ${DLIB_BUILD_TYPE} DDLIB_USE_BLAS=OFF -DDLIB_USE_LAPACK=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                        PREFIX ${CMAKE_BINARY_DIR}/dependencies
                        INSTALL_DIR ${INSTALL_DIR}
                        BUILD_COMMAND ${MAKE}
                        )
            endif ()

            set(dlib_FOUND ON)
            set(dlib_LIB_DIR "${CMAKE_BINARY_DIR}/dependencies/lib")
            set(prefix "lib")
            if (WIN32)
                #            set(suffix "-d.lib")
                set(suffix ".lib")
                set(dlib_LIBRARIES
                        "${dlib_LIB_DIR}/${prefix}dlib${suffix}")
            else ()
                set(suffix ".a")
                set(dlib_LIBRARIES "${dlib_LIB_DIR}/${prefix}dlib${suffix}")
            endif ()
            find_package(GIF QUIET)
            if (GIF_FOUND)
                set(dlib_needed_includes ${dlib_needed_includes} ${GIF_INCLUDE_DIR})
                set(dlib_needed_libraries ${dlib_needed_libraries} ${GIF_LIBRARY})
            endif ()
            find_package(PNG QUIET)
            if (PNG_FOUND)
                set(dlib_needed_includes ${dlib_needed_includes} ${PNG_INCLUDE_DIR})
                set(dlib_needed_libraries ${dlib_needed_libraries} ${PNG_LIBRARY})
            endif ()
            find_package(JPEG QUIET)
            if (JPEG_FOUND)
                set(dlib_needed_includes ${dlib_needed_includes} ${JPEG_INCLUDE_DIR})
                set(dlib_needed_libraries ${dlib_needed_libraries} ${JPEG_LIBRARY})
            endif ()
            find_package(OpenMP)
            if (OPENMP_FOUND)
                set(openmp_libraries ${OpenMP_CXX_FLAGS})
                set(dlib_needed_libraries ${dlib_needed_libraries} ${openmp_libraries})
            endif ()


            set(dlib_LIBS ${dlib_LIBRARIES} ${dlib_needed_libraries})


            link_directories(${dlib_LIB_DIR})
            set(dlib_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/dependencies/include/)
            include_directories(${dlib_INCLUDE_DIRS})
            add_definitions(-DBUILDING_LIBDLIB)

            add_dependencies(${libTools_LIBRARIES} dlib)

        endif ()
        if (libTools_LIBRARIES_CMAKE)
        else ()
            target_link_libraries(${libTools_LIBRARIES} ${dlib_LIBRARIES})
        endif ()
        #    ENDIF (dlib_FOUND)
        if (dlib_FOUND)
            if ("${Tools_Other_Project}" STREQUAL "ON")
                message(STATUS "dlib library status:")
                message(STATUS "    version: ${dlib_VERSION}")
                message(STATUS "    libraries: ${dlib_LIBS}")
                message(STATUS "    libraries: ${dlib_LIBRARIES}")
                message(STATUS "    lib_dir: ${dlib_LIB_DIR}")
                message(STATUS "    include path: ${dlib_INCLUDE_DIRS}")
            endif ()
            add_definitions(-DENABLE_DLIB=ON)
        else ()
            set(${dlib_LIBRARIES} "")
        endif ()
    endif ()
ENDIF (ENABLE_DLIB)

