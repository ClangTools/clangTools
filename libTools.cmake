
set(libTools_LIBRARIES Tools)

find_path(libTools_DIR "libTools.cmake" DOC "Root directory of libTools")
if (EXISTS "${libTools_DIR}/libTools.cmake")
    FILE(GLOB libTools_INCLUDE_DIR "${libTools_DIR}/src/*")
    include(${libTools_DIR}/cmake/find_libs.cmake)
else ()
    message(FATAL_ERROR "cannot find libTools")
endif ()