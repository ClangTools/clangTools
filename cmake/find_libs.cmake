
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

find_package(CURL)
IF (CURL_FOUND)
    INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
ELSE (CURL_FOUND)
    option(CURL_ENABLE "option for CURL" ON)
    if (CURL_ENABLE)
        include(ExternalProject)

        if (WIN32)
            if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 16 2019")
                if (${CMAKE_GENERATOR_PLATFORM} STREQUAL "")
                    set(CMAKE_GENERATOR_PLATFORM WIN64)
                endif ()
                set(G_CMAKE_GENERATOR_PLATFORM
                        -G "${CMAKE_GENERATOR}" -A "${CMAKE_GENERATOR_PLATFORM}")
            else ()
                set(G_CMAKE_GENERATOR_PLATFORM
                        -G "${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM}")
            endif ()
            ExternalProject_Add(curl
                    URL https://github.com/curl/curl/archive/curl-7_67_0.tar.gz
                    URL_MD5 "90b6c61cf3a96a11494deae2f1b3fa92"
                    CONFIGURE_COMMAND cmake ${G_CMAKE_GENERATOR_PLATFORM} -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/dependencies -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF -DSTACK_DIRECTION=-1 -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} <SOURCE_DIR>
                    PREFIX ${CMAKE_BINARY_DIR}/dependencies
                    INSTALL_DIR ${INSTALL_DIR}
                    BUILD_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/curl-build"
                    INSTALL_COMMAND cmake --build "${CMAKE_BINARY_DIR}/dependencies/src/curl-build" --target install
                    )
        else ()
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
        set(CURL_FOUND ON)
    ENDIF (CURL_ENABLE)
ENDIF (CURL_FOUND)
if (CURL_FOUND)
    add_definitions(-DCURL_ENABLE=ON)
else ()
    set(${CURL_LIBRARIES} "")
endif ()
