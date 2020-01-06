
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