
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
    add_compile_options(/wd4244)
    add_compile_options(/wd4251)
    add_compile_options(/wd4819)
    add_compile_options(/wd4091)
    add_compile_options(/wd4996)
    # TARGET_COMPILE_OPTIONS
    ADD_COMPILE_OPTIONS(/MT)

    ADD_COMPILE_OPTIONS(/O1)
    option(DISABLE_OPTIMIZATION "option for DISABLE OPTIMIZATION" OFF)
    if (DISABLE_OPTIMIZATION)
        ADD_COMPILE_OPTIONS(/Od)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG ")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DEBUG ")
        set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} /DEBUG ")
        set(CMAKE_LINK_LIBRARY_FLAG "${CMAKE_LINK_LIBRARY_FLAG} /DEBUG ")
        # /Zi 用来生成程序数据库格式的调试信息   /DEBUG 用来在链接时支持调试
        ADD_DEFINITIONS( /Zi )
        ADD_DEFINITIONS( /DEBUG )
        # SET (LINK_OPTIONS "${LINK_OPTIONS} /DEBUG ")
        SET (LINK_FLAGS "${LINK_FLAGS} /DEBUG ")
        # SET (LIBRARY_LINK_FLAGS "${LIBRARY_LINK_FLAGS} /DEBUG ")
    endif ()

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        add_compile_options(/W0)
    endif ()
endif (MSVC)