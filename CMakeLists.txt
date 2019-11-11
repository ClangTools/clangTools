cmake_minimum_required(VERSION 3.0)
project(Tools)
set(CMAKE_CXX_STANDARD 14)

string(TIMESTAMP time_now_y %y UTC)
string(TIMESTAMP time_now %m%d UTC)
set(FILEVERSION_1 1)
set(FILEVERSION_2 1)
set(FILEVERSION_3 ${time_now_y})
set(FILEVERSION_4 ${time_now})

add_definitions(
        -DLIB_FILEVERSION_1=${FILEVERSION_1}
        -DLIB_FILEVERSION_2=${FILEVERSION_2}
        -DLIB_FILEVERSION_3=${FILEVERSION_3}
        -DLIB_FILEVERSION_4=${FILEVERSION_4}
        -DLIB_FILEVERSION_ALL="${FILEVERSION_1}.${FILEVERSION_2}.${FILEVERSION_3}.${FILEVERSION_4}"
        -DLIB_PROJECT_NAME="${CMAKE_PROJECT_NAME}.dll"
)



# It should be set to 1 for the static MFC library, and 2 for the shared MFC library.
set(CMAKE_MFC_FLAG 1)
#add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-D_AFXDLL)

option(ENABLE_OPENCV "option for OpenCV" OFF)
option(OpenCV_DIR "option for OpenCV DIR" E:\\OpenCvLib\\build_WIN32\\x86\\vc16\\lib)
if (ENABLE_OPENCV)
    add_definitions(-DENABLE_OPENCV)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})

endif ()
include_directories(
        .
        src/
)

aux_source_directory(src srcSource)

if(WIN32)
    FILE(GLOB RC_FILES "./src/*.rc")
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_library(${PROJECT_NAME} STATIC
        ${RC_FILES}
        ${srcSource}
        )

target_compile_definitions(
        ${PROJECT_NAME}
        PUBLIC -D_${PROJECT_NAME}_HEADER_
)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

add_executable(TestDemo Example/main.cpp)

target_link_libraries(TestDemo ${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD        # Adds a post-build event to MyTest
        COMMAND ${CMAKE_COMMAND} -E copy_directory   # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/Depend"      # <--this is in-file
        "${CMAKE_CURRENT_BINARY_DIR}"
        )                 # <--this is out-file path

message("-- instal to " ${CMAKE_INSTALL_PREFIX})

########安装脚本##########
# 将生成的target安装到指定目录下，这里 HLST_Read_Device 是动态库,Example 是可执行程序,
# cmake会自动根据target的类型将可执行程序(如.exe)和动态库(如.dll)作为RUNTIME类型复制到bin下,
# 将动态库的导入库(.lib .a)作为ARCHIVE类型复制lib/static目录下
install(TARGETS ${PROJECT_NAME} TestDemo
        RUNTIME DESTINATION lib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(TARGETS ${PROJECT_NAME} TestDemo
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin)

#install(FILES ${PROJECT_NAME}/${PROJECT_NAME}.h DESTINATION include)

# 复制 bin 代码到 bin 下
install(DIRECTORY Depend/ DESTINATION bin
        PATTERN "Depend/*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ)
# 复制 bin 代码到 bin 下
install(DIRECTORY Depend/ DESTINATION lib
        PATTERN "Depend/*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ)
# 复制 lib 代码到 lib 下
install(DIRECTORY Example/ DESTINATION Example
        PATTERN "Example/*"
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        GROUP_EXECUTE GROUP_READ)
